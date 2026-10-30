/**
 * @file axk_bsp_i2c.c
 * @brief axk BSP I2C 实现（GPIO 软件模拟）
 *
 * 两条独立的软件 I2C 总线：
 *   · s_soft —— 给 CH224A / SHT3x 的 start/stop/... 时序接口
 *   · s_hard —— 给 INA226 的事务式 init/write/read 接口
 * 它们引脚不同、半周期不同，互不影响。
 *
 * SDA 用“输入上拉 = 释放 / 推挽输出低 = 拉低”模拟开漏，
 * 只有需要改变方向时才重新配置 GPIO（bl616cl 每次改方向都会
 * deinit+init，频繁切换很贵）。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_i2c.h"

#include <stddef.h>

#include "axk_bsp_config.h"
#include "axk_bsp_delay.h"
#include "axk_bsp_gpio.h"

/** @brief 一条软件 I2C 总线的运行状态 */
typedef struct {
    uint16_t scl;       /**< SCL 逻辑引脚号 */
    uint16_t sda;       /**< SDA 逻辑引脚号 */
    uint32_t half_us;   /**< 半周期（us） */
    int8_t   sda_out;   /**< SDA 方向缓存：-1 未知，0 已释放，1 已拉低 */
    uint8_t  inited;    /**< 是否已初始化 */
} axk_i2c_bus_t;

static axk_i2c_bus_t s_soft = {
    AXK_BSP_I2C_SOFT_SCL_PIN,
    AXK_BSP_I2C_SOFT_SDA_PIN,
    AXK_BSP_I2C_SOFT_DELAY_US,
    -1, 0
};

static axk_i2c_bus_t s_hard = {
    AXK_BSP_I2C_SCL_PIN,
    AXK_BSP_I2C_SDA_PIN,
    5u,     /* 占位，init 时按 cfg->freq 重算 */
    -1, 0
};

/* ── 基础电平操作 ─────────────────────────────────────────────────── */

static void axk_i2c_sda_release(axk_i2c_bus_t *bus)
{
    if (bus->sda_out != 0) {
        axk_bsp_gpio_init(bus->sda, AXK_BSP_GPIO_MODE_IN_PU);
        bus->sda_out = 0;
    }
}

static void axk_i2c_sda_low(axk_i2c_bus_t *bus)
{
    if (bus->sda_out != 1) {
        axk_bsp_gpio_init(bus->sda, AXK_BSP_GPIO_MODE_OUT);
        bus->sda_out = 1;
    }
    axk_bsp_gpio_reset(bus->sda);
}

static void axk_i2c_scl(axk_i2c_bus_t *bus, int level)
{
    if (level) {
        axk_bsp_gpio_set(bus->scl);
    } else {
        axk_bsp_gpio_reset(bus->scl);
    }
}

static void axk_i2c_delay(axk_i2c_bus_t *bus)
{
    axk_bsp_delay_us(bus->half_us);
}

/* ── 软时序原语 ───────────────────────────────────────────────────── */

static void axk_i2c_start(axk_i2c_bus_t *bus)
{
    /* 起始：SCL 高时 SDA 由高变低 */
    axk_i2c_sda_release(bus);
    axk_i2c_scl(bus, 1);
    axk_i2c_delay(bus);

    axk_i2c_sda_low(bus);
    axk_i2c_delay(bus);

    axk_i2c_scl(bus, 0);
    axk_i2c_delay(bus);
}

static void axk_i2c_stop(axk_i2c_bus_t *bus)
{
    /* 停止：SCL 高时 SDA 由低变高 */
    axk_i2c_sda_low(bus);
    axk_i2c_delay(bus);

    axk_i2c_scl(bus, 1);
    axk_i2c_delay(bus);

    axk_i2c_sda_release(bus);
    axk_i2c_delay(bus);
}

static int axk_i2c_wait_ack(axk_i2c_bus_t *bus)
{
    int ack;

    axk_i2c_sda_release(bus);
    axk_i2c_delay(bus);

    axk_i2c_scl(bus, 1);
    axk_i2c_delay(bus);

    ack = (axk_bsp_gpio_read(bus->sda) == 0) ? 0 : 1;

    axk_i2c_scl(bus, 0);
    axk_i2c_delay(bus);

    return ack;
}

static void axk_i2c_send_byte(axk_i2c_bus_t *bus, uint8_t data)
{
    int8_t i;

    for (i = 7; i >= 0; i--) {
        axk_i2c_scl(bus, 0);
        axk_i2c_delay(bus);

        if (data & (uint8_t)(1u << i)) {
            axk_i2c_sda_release(bus);
        } else {
            axk_i2c_sda_low(bus);
        }
        axk_i2c_delay(bus);

        axk_i2c_scl(bus, 1);
        axk_i2c_delay(bus);
    }

    axk_i2c_scl(bus, 0);
    axk_i2c_delay(bus);
}

static uint8_t axk_i2c_read_byte(axk_i2c_bus_t *bus)
{
    uint8_t data = 0u;
    int8_t i;

    axk_i2c_sda_release(bus);

    for (i = 7; i >= 0; i--) {
        axk_i2c_scl(bus, 0);
        axk_i2c_delay(bus);

        axk_i2c_scl(bus, 1);
        axk_i2c_delay(bus);

        data = (uint8_t)((data << 1) | (uint8_t)(axk_bsp_gpio_read(bus->sda) ? 1 : 0));
    }

    axk_i2c_scl(bus, 0);
    axk_i2c_delay(bus);

    return data;
}

static void axk_i2c_send_ack(axk_i2c_bus_t *bus, int ack)
{
    axk_i2c_scl(bus, 0);
    axk_i2c_delay(bus);

    if (ack) {
        axk_i2c_sda_release(bus);   /* NACK */
    } else {
        axk_i2c_sda_low(bus);       /* ACK */
    }
    axk_i2c_delay(bus);

    axk_i2c_scl(bus, 1);
    axk_i2c_delay(bus);

    axk_i2c_scl(bus, 0);
    axk_i2c_delay(bus);

    /* 释放总线，便于紧接着发 start / stop */
    axk_i2c_sda_release(bus);
}

/* ── 总线初始化 ───────────────────────────────────────────────────── */

static void axk_i2c_bus_setup(axk_i2c_bus_t *bus)
{
    axk_bsp_gpio_init(bus->scl, AXK_BSP_GPIO_MODE_OUT);
    axk_i2c_scl(bus, 1);

    bus->sda_out = -1;
    axk_i2c_sda_release(bus);

    bus->inited = 1u;
}

int axk_bsp_i2c_init(const char *name, bsp_i2c_cfg_t *cfg)
{
    uint32_t freq;
    uint32_t half_us;

    (void)name;     /* axk 只维护一条事务式总线 */

    if (cfg == NULL) {
        return -1;
    }

    freq = (cfg->freq != 0u) ? cfg->freq : AXK_BSP_I2C_FREQ;
    if (freq < 1000u) {
        freq = 1000u;   /* 防止后面的除法得到 0 以外的怪值 */
    }

    half_us = 500000u / freq;
    if (half_us == 0u) {
        half_us = 1u;
    }
    s_hard.half_us = half_us;

    axk_i2c_bus_setup(&s_hard);
    return 0;
}

int axk_bsp_i2c_init_ex(const char *name, uint16_t scl, uint16_t sda,
                        bsp_i2c_cfg_t *cfg)
{
    if (cfg == NULL) {
        return -1;
    }

    s_hard.scl    = scl;
    s_hard.sda    = sda;
    s_hard.inited = 0u;

    return axk_bsp_i2c_init(name, cfg);
}

/* ── 事务式接口 ───────────────────────────────────────────────────── */

int axk_bsp_i2c_write(const char *name, uint8_t addr, const uint8_t *data, uint32_t len)
{
    uint32_t i;

    (void)name;

    if (s_hard.inited == 0u) {
        return -1;
    }
    if (data == NULL && len != 0u) {
        return -1;
    }

    axk_i2c_start(&s_hard);
    axk_i2c_send_byte(&s_hard, (uint8_t)(addr << 1));

    if (axk_i2c_wait_ack(&s_hard) != 0) {
        axk_i2c_stop(&s_hard);
        return -2;
    }

    for (i = 0u; i < len; i++) {
        axk_i2c_send_byte(&s_hard, data[i]);
        if (axk_i2c_wait_ack(&s_hard) != 0) {
            axk_i2c_stop(&s_hard);
            return -3;
        }
    }

    axk_i2c_stop(&s_hard);
    return 0;
}

int axk_bsp_i2c_read(const char *name, uint8_t addr, uint8_t *data, uint32_t len)
{
    uint32_t i;

    (void)name;

    if (s_hard.inited == 0u) {
        return -1;
    }
    if (data == NULL && len != 0u) {
        return -1;
    }
    if (len == 0u) {
        return 0;
    }

    axk_i2c_start(&s_hard);
    axk_i2c_send_byte(&s_hard, (uint8_t)((addr << 1) | 0x01u));

    if (axk_i2c_wait_ack(&s_hard) != 0) {
        axk_i2c_stop(&s_hard);
        return -2;
    }

    for (i = 0u; i < len; i++) {
        data[i] = axk_i2c_read_byte(&s_hard);
        /* 最后一字节回 NACK，其余回 ACK */
        axk_i2c_send_ack(&s_hard, (i + 1u == len) ? 1 : 0);
    }

    axk_i2c_stop(&s_hard);
    return 0;
}

/* ── 软时序接口（CH224A / SHT3x） ─────────────────────────────────── */

void axk_bsp_i2c_soft_init(void)
{
    if (s_soft.inited != 0u) {
        return;
    }

    axk_i2c_bus_setup(&s_soft);
}

void axk_bsp_i2c_soft_init_ex(uint16_t scl, uint16_t sda, uint32_t delay_us)
{
    s_soft.scl    = scl;
    s_soft.sda    = sda;
    s_soft.half_us = (delay_us != 0u) ? delay_us : AXK_BSP_I2C_SOFT_DELAY_US;
    s_soft.inited = 0u;

    axk_bsp_i2c_soft_init();
}

void axk_bsp_i2c_soft_start(void)
{
    axk_i2c_start(&s_soft);
}

void axk_bsp_i2c_soft_stop(void)
{
    axk_i2c_stop(&s_soft);
}

int axk_bsp_i2c_soft_wait_ack(void)
{
    return axk_i2c_wait_ack(&s_soft);
}

int axk_bsp_i2c_soft_send_byte(uint8_t data)
{
    axk_i2c_send_byte(&s_soft, data);
    return 0;
}

uint8_t axk_bsp_i2c_soft_read_byte(void)
{
    return axk_i2c_read_byte(&s_soft);
}

void axk_bsp_i2c_soft_send_ack(int ack)
{
    axk_i2c_send_ack(&s_soft, ack);
}
