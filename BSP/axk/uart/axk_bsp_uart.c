/**
 * @file axk_bsp_uart.c
 * @brief axk BSP UART 实现（基于 open SDK 中间件 aiio_uart）
 *
 * 设备名 → uart_num / 默认引脚在编译期确定，每次 init 重新初始化
 * 对应串口（中间件内部有 inited 标志，重复调用是安全的）。
 *
 * 注意：AI-Thinker 的 AT 固件默认占用 UART1（不同芯片脚位不同），
 * 用 RD03_V2 / HXD039B2 时请选一个应用没有占用的串口，并在
 * axk_bsp_config.h 里把 AXK_BSP_UARTx_TX/RX_PIN 改成实际接线。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_uart.h"

#include <string.h>

#include "axk_bsp_config.h"
#include "aiio_uart.h"

/** @brief 名字解析结果 */
typedef struct {
    uint8_t  uart_num;
    uint16_t tx_pin;
    uint16_t rx_pin;
} axk_uart_pins_t;

/** @brief 运行时引脚覆盖（按 uart_num 索引，init_ex 设置） */
typedef struct {
    uint16_t tx_pin;
    uint16_t rx_pin;
    uint8_t  enabled;
} axk_uart_override_t;

static axk_uart_override_t s_uart_ovr[3];

static int axk_bsp_uart_name_eq(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0') ? 1 : 0;
}

/**
 * @brief 解析设备名，得到中间件串口号与默认引脚
 *
 * 未知/空名字统一退回 AXK_BSP_UART_FIXED_DEV，保证 legacy 接口可用。
 */
static void axk_bsp_uart_resolve(const char *name, axk_uart_pins_t *out)
{
    int known = 0;

    if (name != NULL && name[0] != '\0') {
        known = axk_bsp_uart_name_eq(name, "uart0");
#if defined(__AIIO_UART1)
        known = known || axk_bsp_uart_name_eq(name, "uart1");
#endif
#if defined(__AIIO_UART2)
        known = known || axk_bsp_uart_name_eq(name, "uart2");
#endif
    }

    /* NULL / 空串 / 未知名字：统一用固定口 */
    if (!known) {
        name = AXK_BSP_UART_FIXED_DEV;
    }

    if (axk_bsp_uart_name_eq(name, "uart0")) {
        out->uart_num = (uint8_t)AIIO_UART0;
        out->tx_pin   = (uint16_t)AXK_BSP_UART0_TX_PIN;
        out->rx_pin   = (uint16_t)AXK_BSP_UART0_RX_PIN;
        return;
    }

#if defined(__AIIO_UART1)
    if (axk_bsp_uart_name_eq(name, "uart1")) {
        out->uart_num = (uint8_t)AIIO_UART1;
        out->tx_pin   = (uint16_t)AXK_BSP_UART1_TX_PIN;
        out->rx_pin   = (uint16_t)AXK_BSP_UART1_RX_PIN;
        return;
    }
#endif

#if defined(__AIIO_UART2)
    if (axk_bsp_uart_name_eq(name, "uart2")) {
        out->uart_num = (uint8_t)AIIO_UART2;
        out->tx_pin   = (uint16_t)AXK_BSP_UART2_TX_PIN;
        out->rx_pin   = (uint16_t)AXK_BSP_UART2_RX_PIN;
        return;
    }
#endif

    /* 固定口名字不在支持列表里（用户自定义）：退回 uart0 */
    out->uart_num = (uint8_t)AIIO_UART0;
    out->tx_pin   = (uint16_t)AXK_BSP_UART0_TX_PIN;
    out->rx_pin   = (uint16_t)AXK_BSP_UART0_RX_PIN;
}

/**
 * @brief 由 BSP 配置生成中间件配置
 * @return 0 成功，-1 参数错误
 */
static int axk_bsp_uart_build(const char *name, bsp_uart_cfg_t *cfg,
                              aiio_uart_config_t *uc)
{
    axk_uart_pins_t pins;

    if (cfg == NULL || uc == NULL) {
        return -1;
    }

    axk_bsp_uart_resolve(name, &pins);

    /* 运行时的 init_ex 覆盖优先于板级表/芯片默认脚 */
    if (pins.uart_num < 3u && s_uart_ovr[pins.uart_num].enabled != 0u) {
        pins.tx_pin = s_uart_ovr[pins.uart_num].tx_pin;
        pins.rx_pin = s_uart_ovr[pins.uart_num].rx_pin;
    }

    memset(uc, 0, sizeof(*uc));
    uc->uart_num    = pins.uart_num;
    uc->uart_tx_pin = pins.tx_pin;
    uc->uart_rx_pin = pins.rx_pin;
    uc->baud_rate   = (cfg->baudrate != 0u) ? cfg->baudrate : 115200u;
    uc->irq_rx_bk   = NULL;
    uc->dataBits    = AIIO_UART_DATA_BITS_8;
    uc->stopBits    = AIIO_UART_STOP_BITS_1;
    uc->parity      = AIIO_UART_PARITY_NONE;
    uc->flowControl = AIIO_UART_FLOW_CONTROL_NONE;
    uc->in_at_mode  = 0u;

    return 0;
}

int axk_bsp_uart_init(const char *name, bsp_uart_cfg_t *cfg)
{
    aiio_uart_config_t uc;

    if (axk_bsp_uart_build(name, cfg, &uc) != 0) {
        return -1;
    }

    return (int)aiio_uart_init(uc);
}

int axk_bsp_uart_init_ex(const char *name, uint16_t tx_pin, uint16_t rx_pin,
                         bsp_uart_cfg_t *cfg)
{
    axk_uart_pins_t pins;

    if (cfg == NULL) {
        return -1;
    }

    axk_bsp_uart_resolve(name, &pins);
    if (pins.uart_num >= 3u) {
        return -1;
    }

    s_uart_ovr[pins.uart_num].tx_pin  = tx_pin;
    s_uart_ovr[pins.uart_num].rx_pin  = rx_pin;
    s_uart_ovr[pins.uart_num].enabled = 1u;

    return axk_bsp_uart_init(name, cfg);
}

int axk_bsp_uart_deinit(const char *name)
{
    aiio_uart_config_t uc;
    bsp_uart_cfg_t cfg;

    cfg.baudrate = AXK_BSP_UART_FIXED_BAUDRATE;
    if (axk_bsp_uart_build(name, &cfg, &uc) != 0) {
        return -1;
    }

    return (int)aiio_uart_deinit(uc);
}

int axk_bsp_uart_param_check(const char *name, bsp_uart_cfg_t *cfg)
{
    aiio_uart_config_t uc;

    if (axk_bsp_uart_build(name, cfg, &uc) != 0) {
        return -1;
    }

    return (int)aiio_uart_param_check(uc);
}

int axk_bsp_uart_getchar(const char *name)
{
    axk_uart_pins_t pins;
    uint8_t ch = 0u;
    int32_t ret;

    axk_bsp_uart_resolve(name, &pins);

    ret = aiio_uart_recv_data(pins.uart_num, &ch, 1u);

    /*
     * 各芯片实现：BL616CL 返回实际读到的字节数（非阻塞，无数据返回 0），
     * LN882H/RTL 阻塞到读到 1 字节。统一按“返回值 > 0 表示有数据”处理。
     */
    return (ret > 0) ? (int)ch : -1;
}

void axk_bsp_uart_putchar(const char *name, int ch)
{
    axk_uart_pins_t pins;
    uint8_t byte = (uint8_t)ch;

    axk_bsp_uart_resolve(name, &pins);
    (void)aiio_uart_send_data(pins.uart_num, &byte, 1u);
}

/* ── 固定口接口（HXD039B2） ───────────────────────────────────────── */

int axk_bsp_uart_legacy_init(void)
{
    bsp_uart_cfg_t cfg;

    cfg.baudrate = AXK_BSP_UART_FIXED_BAUDRATE;
    return axk_bsp_uart_init(AXK_BSP_UART_FIXED_DEV, &cfg);
}

void axk_bsp_uart_legacy_send_byte(uint8_t data)
{
    axk_bsp_uart_putchar(AXK_BSP_UART_FIXED_DEV, (int)data);
}
