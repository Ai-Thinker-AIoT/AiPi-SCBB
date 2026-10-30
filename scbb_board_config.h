/**
 * @file scbb_board_config.h
 * @brief 板级引脚表 —— 换板子时唯一需要手改的引脚配置文件
 *
 * 说明：
 *   · 本文件不会被 menuconfig 重新生成，可以放心手改，也建议跟工程一起提交
 *   · 只写需要变化的引脚，没写的继续使用库里的默认值
 *   · 库里同名宏全部有 #ifndef 保护，这里定义会优先生效
 *   · 优先级：main.c 里显式调用（init_cfg / *_init_ex）> 本文件 > 库默认值
 *
 * 什么时候用哪一层：
 *   · 总线脚（SPI/I2C/UART）：整块板共享，在本文件配置一次即可；
 *     如果想在 main.c 里动态指定，调用 BSP 的 *_init_ex() 接口
 *   · 模块脚（CS/DC/RST/DATA/BUSY...）：本文件可改默认值；
 *     同一块板挂多个同型号设备时，请在 main.c 用模块的 init_cfg() 分别指定
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 */
#ifndef SCBB_BOARD_CONFIG_H
#define SCBB_BOARD_CONFIG_H

/* ── 软件 SPI 总线（SPI0 默认 13/11/11，SPI1 默认 5/19/19） ───────── */
/* #define AXK_BSP_SPI0_SCLK_PIN   13 */
/* #define AXK_BSP_SPI0_MOSI_PIN   11 */
/* #define AXK_BSP_SPI0_MISO_PIN   11 */
/* #define AXK_BSP_SPI1_SCLK_PIN   5 */
/* #define AXK_BSP_SPI1_MOSI_PIN   19 */
/* #define AXK_BSP_SPI1_MISO_PIN   19 */

/* ── 软件 I2C 总线（SHT3x/CH224A 默认 8/18，INA226 默认 4/5） ─────── */
/* #define AXK_BSP_I2C_SOFT_SCL_PIN   8 */
/* #define AXK_BSP_I2C_SOFT_SDA_PIN   18 */
/* #define AXK_BSP_I2C_SOFT_DELAY_US  5 */
/* #define AXK_BSP_I2C_SCL_PIN        4 */
/* #define AXK_BSP_I2C_SDA_PIN        5 */

/* ── UART 总线（默认取芯片 spec 的串口脚，这里写数字逻辑引脚号） ──── */
/* 例如 BL602 的 uart0 默认 TX=16 / RX=7 */
/* #define AXK_BSP_UART0_TX_PIN   16 */
/* #define AXK_BSP_UART0_RX_PIN   7 */
/* #define AXK_BSP_UART1_TX_PIN   4 */
/* #define AXK_BSP_UART1_RX_PIN   5 */

/* ── 模块默认脚（运行时 init_cfg 可覆盖同一模块） ─────────────────── */
/* #define AXK_ST7789V_LCD_PIN_CS     10 */
/* #define AXK_ST7789V_LCD_PIN_DC     1 */
/* #define AXK_ST7789V_LCD_PIN_RST    18 */
/* #define AXK_ST7789V_LCD_PIN_BLK    20 */
/* #define AXK_DHT11_PIN              0 */
/* #define AXK_RELAY_PIN              0 */
/* #define AXK_DS1302_PIN_CE          16 */
/* #define AXK_DS1302_PIN_SCLK        15 */
/* #define AXK_DS1302_PIN_IO          14 */
/* #define AXK_OLED_096_SPI_PIN_CS    10 */
/* #define AXK_OLED_096_SPI_PIN_DC    1 */
/* #define AXK_OLED_096_SPI_PIN_RST   18 */
/* #define AXK_HXD039B2_BUSY_PIN      1 */
/* #define AXK_LLCC68_NSS_PIN         4 */
/* #define AXK_LLCC68_NRESET_PIN      0x10E */
/* #define AXK_LLCC68_BUSY_PIN        0 */
/* #define AXK_LLCC68_DIO1_PIN        0x101 */

#endif /* SCBB_BOARD_CONFIG_H */
