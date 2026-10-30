/**
 * @file axk_bsp_spi.c
 * @brief axk BSP SPI 实现（GPIO 软件模拟，支持模式 0~3）
 *
 * 为什么不用 aiio 的硬件 SPI：
 *   aiio_spi 的 CS 处理、引脚复用和频率配置在 bl616cl / RTL / LN882H 上
 *   各不相同（有的强制 1MHz，有的只能用固定脚），而 OLED/LLCC68 的速率
 *   要求也不高，软件模拟最省心且全芯片一致。
 *
 * 时序：
 *   mode = {CPOL, CPHA}
 *   CPHA=0：数据在第一个边沿前建立，第一个边沿采样
 *   CPHA=1：数据在第一个边沿上改变，第二个边沿采样
 *   MSB 先行；SCLK 空闲电平 = CPOL
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_spi.h"

#include <stddef.h>

#include "axk_bsp_config.h"
#include "axk_bsp_delay.h"
#include "axk_bsp_gpio.h"

/** @brief 一组软件 SPI 总线 */
typedef struct {
    uint16_t sclk;
    uint16_t mosi;
    uint16_t miso;
    uint32_t half_us;   /**< 半周期延时（0 = 不插延时） */
    uint8_t  mode;      /**< 0~3 */
    uint8_t  inited;
} axk_spi_bus_t;

static axk_spi_bus_t s_spi_bus[2] = {
    {
        AXK_BSP_SPI0_SCLK_PIN,
        AXK_BSP_SPI0_MOSI_PIN,
        AXK_BSP_SPI0_MISO_PIN,
        0u, 0u, 0u
    },
    {
        AXK_BSP_SPI1_SCLK_PIN,
        AXK_BSP_SPI1_MOSI_PIN,
        AXK_BSP_SPI1_MISO_PIN,
        0u, 0u, 0u
    },
};

static axk_spi_bus_t *axk_spi_get_bus(const char *name)
{
    if (name != NULL &&
        name[0] == 's' && name[1] == 'p' && name[2] == 'i' &&
        name[3] == '1' && name[4] == '\0') {
        return &s_spi_bus[1];
    }
    return &s_spi_bus[0];
}

static void axk_spi_delay(const axk_spi_bus_t *bus)
{
    if (bus->half_us != 0u) {
        axk_bsp_delay_us(bus->half_us);
    }
}

static void axk_spi_set_sclk(const axk_spi_bus_t *bus, int level)
{
    if (level) {
        axk_bsp_gpio_set(bus->sclk);
    } else {
        axk_bsp_gpio_reset(bus->sclk);
    }
}

static void axk_spi_set_mosi(const axk_spi_bus_t *bus, int level)
{
    if (level) {
        axk_bsp_gpio_set(bus->mosi);
    } else {
        axk_bsp_gpio_reset(bus->mosi);
    }
}

static axk_spi_bus_t *axk_spi_ensure(const char *name)
{
    axk_spi_bus_t *bus = axk_spi_get_bus(name);

    if (bus->inited == 0u) {
        bsp_spi_cfg_t cfg = { 1000000u, 0u, 8u };

        (void)axk_bsp_spi_init(name, &cfg);
    }
    return bus;
}

static uint8_t axk_spi_xfer(axk_spi_bus_t *bus, uint8_t tx)
{
    uint8_t rx = 0u;
    uint8_t idle = (bus->mode & 0x02u) ? 1u : 0u;
    uint8_t cpha = (bus->mode & 0x01u) ? 1u : 0u;
    int8_t  i;

    for (i = 7; i >= 0; i--) {
        uint8_t bit = (uint8_t)((tx >> i) & 0x01u);

        if (!cpha) {
            /* CPHA=0：第一个边沿前建立数据 */
            axk_spi_set_mosi(bus, bit);
            axk_spi_delay(bus);
        }

        /* 前沿：SCLK 离开空闲电平 */
        axk_spi_set_sclk(bus, idle ? 0 : 1);
        axk_spi_delay(bus);

        if (cpha) {
            /* CPHA=1：第一个边沿上改变数据 */
            axk_spi_set_mosi(bus, bit);
            axk_spi_delay(bus);
        } else {
            /* CPHA=0：第一个边沿采样 */
            rx = (uint8_t)((rx << 1) | (uint8_t)(axk_bsp_gpio_read(bus->miso) ? 1 : 0));
        }

        /* 后沿：SCLK 回到空闲电平 */
        axk_spi_set_sclk(bus, idle ? 1 : 0);
        axk_spi_delay(bus);

        if (cpha) {
            /* CPHA=1：第二个边沿采样 */
            rx = (uint8_t)((rx << 1) | (uint8_t)(axk_bsp_gpio_read(bus->miso) ? 1 : 0));
        }
    }

    return rx;
}

int axk_bsp_spi_init(const char *name, bsp_spi_cfg_t *cfg)
{
    axk_spi_bus_t *bus;
    uint32_t freq;
    uint8_t idle;

    if (cfg == NULL) {
        return -1;
    }

    bus  = axk_spi_get_bus(name);
    freq = (cfg->freq != 0u) ? cfg->freq : 1000000u;
    bus->mode = (uint8_t)(cfg->mode & 0x03u);
    bus->half_us = (freq > 0u) ? (500000u / freq) : 0u;

    idle = (bus->mode & 0x02u) ? 1u : 0u;

    /* SCLK 先进入空闲电平，避免从机看到毛刺 */
    axk_bsp_gpio_init(bus->sclk, AXK_BSP_GPIO_MODE_OUT);
    axk_spi_set_sclk(bus, idle);

    /* MOSI 输出低；MISO 若与 MOSI 同脚则只配置一次 */
    axk_bsp_gpio_init(bus->mosi, AXK_BSP_GPIO_MODE_OUT);
    axk_spi_set_mosi(bus, 0);

    if (bus->miso != bus->mosi) {
        axk_bsp_gpio_init(bus->miso, AXK_BSP_GPIO_MODE_IN_PU);
    }

    bus->inited = 1u;
    return 0;
}

int axk_bsp_spi_init_ex(const char *name, uint16_t sclk, uint16_t mosi,
                        uint16_t miso, bsp_spi_cfg_t *cfg)
{
    axk_spi_bus_t *bus;

    if (cfg == NULL) {
        return -1;
    }

    /* 先写入引脚，再走常规 init：后续模块 init 会复用这组引脚 */
    bus = axk_spi_get_bus(name);
    bus->sclk   = sclk;
    bus->mosi   = mosi;
    bus->miso   = miso;
    bus->inited = 0u;

    return axk_bsp_spi_init(name, cfg);
}

void axk_bsp_spi_send8(const char *name, uint8_t data)
{
    axk_spi_bus_t *bus = axk_spi_ensure(name);

    (void)axk_spi_xfer(bus, data);
}

void axk_bsp_spi_send16(const char *name, const uint16_t *data, uint32_t n)
{
    axk_spi_bus_t *bus;
    uint32_t i;

    if (data == NULL) {
        return;
    }

    bus = axk_spi_ensure(name);

    for (i = 0u; i < n; i++) {
        (void)axk_spi_xfer(bus, (uint8_t)(data[i] >> 8));
        (void)axk_spi_xfer(bus, (uint8_t)(data[i] & 0xFFu));
    }
}

uint8_t axk_bsp_spi_transfer8(const char *name, uint8_t data)
{
    axk_spi_bus_t *bus = axk_spi_ensure(name);

    return axk_spi_xfer(bus, data);
}
