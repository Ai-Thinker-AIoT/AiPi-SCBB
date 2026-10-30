/**
 * @file axk_bsp_uart.h
 * @brief axk BSP UART 接口（基于 open SDK 中间件 aiio_uart）
 *
 * 两套接口：
 *   1. 带设备名（RD03_V2 用）
 *        axk_bsp_uart_init(name, cfg) / getchar(name) / putchar(name, ch)
 *   2. 固定口（HXD039B2 用，模块只传数据不传名字）
 *        axk_bsp_uart_legacy_init() / axk_bsp_uart_legacy_send_byte(byte)
 *
 * 设备名 "uart0"/"uart1"/"uart2"，默认引脚取芯片 spec 的
 * AIIO_UARTn_TX/RX，可在 ../axk_bsp_config.h 覆盖。
 *
 * ⚠ UART 的引脚参数是**中间件原生 pin 值**（如 AIIO_GPIO_NUM_5），
 *   不是 GPIO 那套 0x100|n 端口编码——aiio_uart_config_t 没有 port 字段，
 *   端口由各芯片的实现按 pin 号查表决定。
 *
 * ⚠ aiio_uart_recv_data() 的行为各芯片不同（BL 非阻塞、LN882H/RTL 阻塞），
 *   getchar() 无法在所有芯片上保证非阻塞，详见 README。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_UART_H
#define AXK_BSP_UART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BSP_UART_CFG_T_DEFINED
#define BSP_UART_CFG_T_DEFINED
/** @brief UART 配置结构体（与 Ai-M6x BSP 保持一致） */
typedef struct {
    uint32_t baudrate;  /**< 波特率（如 115200） */
} bsp_uart_cfg_t;
#endif

/**
 * @brief 初始化 UART（8N1，无流控）
 * @param name 设备名："uart0"/"uart1"/"uart2"；NULL 或未知名字用固定口
 * @param cfg  配置，NULL 返回 -1
 * @return 0 成功，其他为中间件错误码
 */
int axk_bsp_uart_init(const char *name, bsp_uart_cfg_t *cfg);

/**
 * @brief 初始化 UART 并显式指定 TX/RX 引脚（运行时可改，优先于板级表）
 *
 * 引脚覆盖按串口号保存，之后同一串口的 init/deinit/param_check 都会复用。
 *
 * @param name   设备名："uart0"/"uart1"/"uart2"
 * @param tx_pin TX 引脚（axk 逻辑引脚号）
 * @param rx_pin RX 引脚（axk 逻辑引脚号）
 * @param cfg    配置，NULL 返回 -1
 * @return 0 成功，其他为中间件错误码
 */
int axk_bsp_uart_init_ex(const char *name, uint16_t tx_pin, uint16_t rx_pin,
                         bsp_uart_cfg_t *cfg);

/**
 * @brief 接收一个字节
 * @return 0~255 = 收到的字节，-1 = 暂无数据
 * @note   LN882H / RTL 的中间件实现会阻塞，调用方注意
 */
int axk_bsp_uart_getchar(const char *name);

/** @brief 发送一个字节 */
void axk_bsp_uart_putchar(const char *name, int ch);

/**
 * @brief 反初始化 UART
 * @return 0 成功，其他为中间件错误码
 */
int axk_bsp_uart_deinit(const char *name);

/**
 * @brief 校验 UART 参数是否被当前芯片支持（不实际初始化）
 * @return 0 合法，其他为中间件错误码
 */
int axk_bsp_uart_param_check(const char *name, bsp_uart_cfg_t *cfg);

/* ── 固定口接口（HXD039B2） ───────────────────────────────────────── */

/** @brief 按 AXK_BSP_UART_FIXED_DEV / _BAUDRATE 初始化固定串口 */
int axk_bsp_uart_legacy_init(void);

/** @brief 向固定串口发送一个字节 */
void axk_bsp_uart_legacy_send_byte(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* AXK_BSP_UART_H */
