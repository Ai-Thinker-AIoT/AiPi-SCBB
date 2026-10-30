/**
 * @file axk_bsp_config.h
 * @brief axk BSP —— 板级参数与插桩点（基于 open SDK 中间件 aiio_*）
 *
 * 本文件是 BSP/axk 的唯一配置入口：
 *   · 引脚/总线参数：在 scbb_board_config.h 里 #define 同名宏即可覆盖（全部有 #ifndef）
 *   · GPIO 编码：把 SCBB 模块里的裸引脚号翻译成中间件的 (port, pin) 对
 *
 * ── 引脚编码 ──────────────────────────────────────────────────────────
 * 中间件 aiio_hal_gpio_* 需要 (port, pin) 两个参数，而 SCBB 模块只传一个
 * uint16_t 引脚号。axk 的约定：
 *
 *     bit[8:9]  端口号：0 = AIIO_GPIO_A，1 = AIIO_GPIO_B，2 = AIIO_GPIO_C
 *     bit[7:0]  端口内引脚序号（不是位掩码，映射由 axk_bsp_gpio_num_of 完成）
 *
 *   · 0x00 ~ 0xFF   → 端口 A，兼容所有 BL 系列（BL616CL 的 IO18/IO40 直接写 18/40）
 *   · 0x100 | n     → 端口 B（如 RTL8711 的 PB31 写 (1 << 8) | 31）
 *   · 0x200 | n     → 端口 C（仅 rtl8711fx 有 C 口）
 *
 * 也可以直接用宏：AXK_BSP_GPIO_PIN(1, 31) == 0x11F。
 *
 * 中间件对“pin”的表示并不统一：BL/BS21/RTL 是纯序号，LN882H 是
 * GPIO_PIN_n 位掩码。axk_bsp_gpio_num_of() 用 __AIIO_GPIO_NUM_n 宏做了
 * 逐颗芯片的静态映射，模块侧不用关心。
 *
 * ── 使用前提 ──────────────────────────────────────────────────────────
 * 1. 本 BSP 只能编译在 open SDK（aithinker_dev_open_sdk）工程里，
 *    aiio_gpio.h / aiio_uart.h / aiio_os_port.h 由中间件提供。
 * 2. 上电后、进入 taskENTER_CRITICAL() 之前调用一次 axk_bsp_delay_init()，
 *    否则 us 级延时只能用估算值（见 delay/axk_bsp_delay.c）。
 * 3. 模块侧引脚宏（AXK_DHT11_PIN 等）必须与这里的总线引脚一致，
 *    因为 BSP 的 init(name, cfg) 接口拿不到引脚参数。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_CONFIG_H
#define AXK_BSP_CONFIG_H

#include <stdint.h>

#include "aiio_gpio.h"
#include "aiio_uart.h"

/*
 * 板级引脚表：存在即覆盖下面的默认值。
 * 用户手改 scbb_board_config.h，不参与 menuconfig 生成，不会被覆盖。
 */
#if defined(__has_include)
#if __has_include("scbb_board_config.h")
#include "scbb_board_config.h"
#endif
#endif

/*
 * menuconfig 生成的引脚（scbb_config.h）。
 * BSP 源文件不包含模块头，不引它的话，下面的 #ifndef 默认值会先定义，
 * menuconfig / 板级预设选的引脚就失效了。
 */
#if defined(__has_include)
#if __has_include("scbb_config.h")
#include "scbb_config.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * 通用返回码 / 弱符号
 * ========================================================================== */

#define AXK_BSP_OK               (0)
#define AXK_BSP_ERR              (-1)   /*!< 中间件返回失败 */
#define AXK_BSP_ERR_PARAM        (-2)   /*!< 参数错误 */
#define AXK_BSP_ERR_UNSUPPORTED  (-3)   /*!< 当前芯片未实现该 aiio API */

/*
 * 中间件头文件声明的 API 并不在每颗芯片上都实现（例如 I2S 只有 BL 系有、
 * PWM 捕获只有 BL602/LN882H 有、部分 flash 接口所有芯片都没实现）。
 * open SDK 的组件构建会把 BSP/axk 下所有 .c 一起编进应用，直接引用这些
 * 符号会在缺失芯片上链接失败。因此外设包装层统一用弱引用来调用：
 *
 *     extern int aiio_xxx(...) __attribute__((weak));
 *     if (aiio_xxx == NULL) return AXK_BSP_ERR_UNSUPPORTED;
 *
 * 有实现的芯片正常链接到强符号，没有实现时弱符号为 NULL，运行时返回
 * AXK_BSP_ERR_UNSUPPORTED，不会破坏整个工程的链接。
 */
#if defined(__GNUC__) || defined(__clang__)
#define AXK_BSP_WEAK                    __attribute__((weak))
#define AXK_BSP_WEAK_AVAILABLE(fn)      ((fn) != NULL)
#else
#define AXK_BSP_WEAK
#define AXK_BSP_WEAK_AVAILABLE(fn)      (1)
#endif

/* ==========================================================================
 * GPIO 引脚翻译
 * ========================================================================== */

/** @brief 拼一个 axk 逻辑引脚号（port: 0=A, 1=B, 2=C） */
#define AXK_BSP_GPIO_PIN(port, pin) \
    ((uint16_t)((((uint32_t)(port) & 0x03u) << 8) | ((uint32_t)(pin) & 0xFFu)))

/**
 * @brief 逻辑引脚号 → 中间件端口号
 *
 * AIIO_GPIO_A/B 在每颗芯片的 spec 里都有定义；C 口只有 rtl8711fx 有。
 */
static inline uint32_t axk_bsp_gpio_port_of(uint16_t pin)
{
    uint32_t port = ((uint32_t)pin >> 8) & 0x03u;

    if (port == 1u) {
        return (uint32_t)AIIO_GPIO_B;
    }
    if (port == 2u) {
#ifdef __AIIO_GPIO_C
        return (uint32_t)AIIO_GPIO_C;
#else
        return (uint32_t)AIIO_GPIO_A;   /* 该芯片没有 C 口，退回 A */
#endif
    }
    return (uint32_t)AIIO_GPIO_A;
}

/**
 * @brief 逻辑引脚号 → 中间件引脚号
 *
 * 绝大多数芯片 __AIIO_GPIO_NUM_n == n；LN882H 是 GPIO_PIN_n 位掩码，
 * 这里用 #ifdef 逐个映射（未定义的序号走 default 原样返回）。
 */
static inline uint16_t axk_bsp_gpio_num_of(uint16_t pin)
{
    uint16_t idx = (uint16_t)(pin & 0xFFu);

    switch (idx) {
#ifdef __AIIO_GPIO_NUM_0
    case 0:  return (uint16_t)AIIO_GPIO_NUM_0;
#endif
#ifdef __AIIO_GPIO_NUM_1
    case 1:  return (uint16_t)AIIO_GPIO_NUM_1;
#endif
#ifdef __AIIO_GPIO_NUM_2
    case 2:  return (uint16_t)AIIO_GPIO_NUM_2;
#endif
#ifdef __AIIO_GPIO_NUM_3
    case 3:  return (uint16_t)AIIO_GPIO_NUM_3;
#endif
#ifdef __AIIO_GPIO_NUM_4
    case 4:  return (uint16_t)AIIO_GPIO_NUM_4;
#endif
#ifdef __AIIO_GPIO_NUM_5
    case 5:  return (uint16_t)AIIO_GPIO_NUM_5;
#endif
#ifdef __AIIO_GPIO_NUM_6
    case 6:  return (uint16_t)AIIO_GPIO_NUM_6;
#endif
#ifdef __AIIO_GPIO_NUM_7
    case 7:  return (uint16_t)AIIO_GPIO_NUM_7;
#endif
#ifdef __AIIO_GPIO_NUM_8
    case 8:  return (uint16_t)AIIO_GPIO_NUM_8;
#endif
#ifdef __AIIO_GPIO_NUM_9
    case 9:  return (uint16_t)AIIO_GPIO_NUM_9;
#endif
#ifdef __AIIO_GPIO_NUM_10
    case 10: return (uint16_t)AIIO_GPIO_NUM_10;
#endif
#ifdef __AIIO_GPIO_NUM_11
    case 11: return (uint16_t)AIIO_GPIO_NUM_11;
#endif
#ifdef __AIIO_GPIO_NUM_12
    case 12: return (uint16_t)AIIO_GPIO_NUM_12;
#endif
#ifdef __AIIO_GPIO_NUM_13
    case 13: return (uint16_t)AIIO_GPIO_NUM_13;
#endif
#ifdef __AIIO_GPIO_NUM_14
    case 14: return (uint16_t)AIIO_GPIO_NUM_14;
#endif
#ifdef __AIIO_GPIO_NUM_15
    case 15: return (uint16_t)AIIO_GPIO_NUM_15;
#endif
#ifdef __AIIO_GPIO_NUM_16
    case 16: return (uint16_t)AIIO_GPIO_NUM_16;
#endif
#ifdef __AIIO_GPIO_NUM_17
    case 17: return (uint16_t)AIIO_GPIO_NUM_17;
#endif
#ifdef __AIIO_GPIO_NUM_18
    case 18: return (uint16_t)AIIO_GPIO_NUM_18;
#endif
#ifdef __AIIO_GPIO_NUM_19
    case 19: return (uint16_t)AIIO_GPIO_NUM_19;
#endif
#ifdef __AIIO_GPIO_NUM_20
    case 20: return (uint16_t)AIIO_GPIO_NUM_20;
#endif
#ifdef __AIIO_GPIO_NUM_21
    case 21: return (uint16_t)AIIO_GPIO_NUM_21;
#endif
#ifdef __AIIO_GPIO_NUM_22
    case 22: return (uint16_t)AIIO_GPIO_NUM_22;
#endif
#ifdef __AIIO_GPIO_NUM_23
    case 23: return (uint16_t)AIIO_GPIO_NUM_23;
#endif
#ifdef __AIIO_GPIO_NUM_24
    case 24: return (uint16_t)AIIO_GPIO_NUM_24;
#endif
#ifdef __AIIO_GPIO_NUM_25
    case 25: return (uint16_t)AIIO_GPIO_NUM_25;
#endif
#ifdef __AIIO_GPIO_NUM_26
    case 26: return (uint16_t)AIIO_GPIO_NUM_26;
#endif
#ifdef __AIIO_GPIO_NUM_27
    case 27: return (uint16_t)AIIO_GPIO_NUM_27;
#endif
#ifdef __AIIO_GPIO_NUM_28
    case 28: return (uint16_t)AIIO_GPIO_NUM_28;
#endif
#ifdef __AIIO_GPIO_NUM_29
    case 29: return (uint16_t)AIIO_GPIO_NUM_29;
#endif
#ifdef __AIIO_GPIO_NUM_30
    case 30: return (uint16_t)AIIO_GPIO_NUM_30;
#endif
#ifdef __AIIO_GPIO_NUM_31
    case 31: return (uint16_t)AIIO_GPIO_NUM_31;
#endif
#ifdef __AIIO_GPIO_NUM_32
    case 32: return (uint16_t)AIIO_GPIO_NUM_32;
#endif
#ifdef __AIIO_GPIO_NUM_33
    case 33: return (uint16_t)AIIO_GPIO_NUM_33;
#endif
#ifdef __AIIO_GPIO_NUM_34
    case 34: return (uint16_t)AIIO_GPIO_NUM_34;
#endif
#ifdef __AIIO_GPIO_NUM_35
    case 35: return (uint16_t)AIIO_GPIO_NUM_35;
#endif
#ifdef __AIIO_GPIO_NUM_36
    case 36: return (uint16_t)AIIO_GPIO_NUM_36;
#endif
#ifdef __AIIO_GPIO_NUM_37
    case 37: return (uint16_t)AIIO_GPIO_NUM_37;
#endif
#ifdef __AIIO_GPIO_NUM_38
    case 38: return (uint16_t)AIIO_GPIO_NUM_38;
#endif
#ifdef __AIIO_GPIO_NUM_39
    case 39: return (uint16_t)AIIO_GPIO_NUM_39;
#endif
#ifdef __AIIO_GPIO_NUM_40
    case 40: return (uint16_t)AIIO_GPIO_NUM_40;
#endif
    default: return idx;
    }
}

/* ==========================================================================
 * GPIO 模式（与 Ai-M6x BSP 的 BSP_GPIO_MODE_* 取值一致）
 * ========================================================================== */
#define AXK_BSP_GPIO_MODE_IN      0u   /**< 浮空输入 */
#define AXK_BSP_GPIO_MODE_OUT     1u   /**< 推挽输出（2/3 也按输出处理） */
#define AXK_BSP_GPIO_MODE_SPI_MOSI 2u  /**< 兼容旧 BSP：axk 里等同输出 */
#define AXK_BSP_GPIO_MODE_SPI_SCLK 3u  /**< 兼容旧 BSP：axk 里等同输出 */
#define AXK_BSP_GPIO_MODE_IN_PU   4u   /**< 上拉输入 */

/* ==========================================================================
 * 软件 I2C（CH224A / SHT3x —— 走 AXK_*_I2C_ACLL 的 start/stop/... 接口）
 * ========================================================================== */
#ifndef AXK_BSP_I2C_SOFT_SCL_PIN
#define AXK_BSP_I2C_SOFT_SCL_PIN   8   /**< 软 I2C 时钟线 */
#endif
#ifndef AXK_BSP_I2C_SOFT_SDA_PIN
#define AXK_BSP_I2C_SOFT_SDA_PIN   18  /**< 软 I2C 数据线 */
#endif
#ifndef AXK_BSP_I2C_SOFT_DELAY_US
#define AXK_BSP_I2C_SOFT_DELAY_US  5   /**< 半周期延时（us），5 ≈ 100kHz */
#endif

/* ==========================================================================
 * 事务式 I2C（INA226 —— 走 AXK_*_I2C_ACLL 的 init/write/read 接口）
 * ========================================================================== */
#ifndef AXK_BSP_I2C_SCL_PIN
#define AXK_BSP_I2C_SCL_PIN        4   /**< I2C 时钟线 */
#endif
#ifndef AXK_BSP_I2C_SDA_PIN
#define AXK_BSP_I2C_SDA_PIN        5   /**< I2C 数据线 */
#endif
#ifndef AXK_BSP_I2C_FREQ
#define AXK_BSP_I2C_FREQ           100000u  /**< 默认 100kHz（cfg->freq 优先） */
#endif

/* ==========================================================================
 * 软件 SPI（OLED096 / LLCC68）
 *
 * 模块按名字选总线："spi0" → 第 0 组，"spi1" → 第 1 组。
 *   · spi0 默认 13/11/11 —— 与 OLED_096_SPI 模块头里的 SCL/SDA 默认一致
 *   · spi1 默认 5/19/19  —— 旧 AiPi-SCBB 对 LLCC68 的接线
 * MISO 与 MOSI 同脚表示“只写不回读”，此时 transfer8 读回的是 MOSI 上的值。
 * ========================================================================== */
#ifndef AXK_BSP_SPI0_SCLK_PIN
#define AXK_BSP_SPI0_SCLK_PIN      13
#endif
#ifndef AXK_BSP_SPI0_MOSI_PIN
#define AXK_BSP_SPI0_MOSI_PIN      11
#endif
#ifndef AXK_BSP_SPI0_MISO_PIN
#define AXK_BSP_SPI0_MISO_PIN      11
#endif

#ifndef AXK_BSP_SPI1_SCLK_PIN
#define AXK_BSP_SPI1_SCLK_PIN      5
#endif
#ifndef AXK_BSP_SPI1_MOSI_PIN
#define AXK_BSP_SPI1_MOSI_PIN      19
#endif
#ifndef AXK_BSP_SPI1_MISO_PIN
#define AXK_BSP_SPI1_MISO_PIN      19
#endif

/* ==========================================================================
 * UART
 *
 * 带设备名接口（RD03_V2）："uart0"/"uart1"/"uart2"，默认引脚取芯片 spec
 * 里该串口的默认脚（__AIIO_UARTx_TX/RX），可在这里覆盖。
 * 固定口接口（HXD039B2）：设备名/波特率固定为下面的宏。
 * ========================================================================== */
#ifndef AXK_BSP_UART0_TX_PIN
#define AXK_BSP_UART0_TX_PIN       AIIO_UART0_TX
#endif
#ifndef AXK_BSP_UART0_RX_PIN
#define AXK_BSP_UART0_RX_PIN       AIIO_UART0_RX
#endif

#if defined(__AIIO_UART1)
#ifndef AXK_BSP_UART1_TX_PIN
#define AXK_BSP_UART1_TX_PIN       AIIO_UART1_TX
#endif
#ifndef AXK_BSP_UART1_RX_PIN
#define AXK_BSP_UART1_RX_PIN       AIIO_UART1_RX
#endif
#endif

#if defined(__AIIO_UART2)
#ifndef AXK_BSP_UART2_TX_PIN
#define AXK_BSP_UART2_TX_PIN       AIIO_UART2_TX
#endif
#ifndef AXK_BSP_UART2_RX_PIN
#define AXK_BSP_UART2_RX_PIN       AIIO_UART2_RX
#endif
#endif

#ifndef AXK_BSP_UART_FIXED_DEV
#define AXK_BSP_UART_FIXED_DEV     "uart1"
#endif
#ifndef AXK_BSP_UART_FIXED_BAUDRATE
#define AXK_BSP_UART_FIXED_BAUDRATE 115200u
#endif

#ifdef __cplusplus
}
#endif

#endif /* AXK_BSP_CONFIG_H */
