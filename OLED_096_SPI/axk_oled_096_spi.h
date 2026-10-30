/**
 * @file axk_oled_096_spi.h
 * @brief 0.96" OLED (SSD1306, 128x64, SPI) — 纯 ACLL 跨平台。
 *
 * 硬件 SPI，帧缓冲渲染。支持像素级绘制、全屏刷新和清屏。
 * CS/DC/RST 是模块自己的脚，可在运行时通过 axk_oled_096_spi_init_cfg()
 * 指定；SCL/SDA 属于 SPI 总线脚，由 BSP 配置（axk 平台见
 * BSP/axk/axk_bsp_spi_init_ex() 或 scbb_board_config.h）。
 * 默认控制脚：CS=IO10, DC=IO1, RST=IO18。
 *
 * @author xemowo (xemowo@qq.com)
 * @version 0.1.1
 * @date 2026-07-31
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_OLED_096_SPI_H
#define AXK_OLED_096_SPI_H

#include "scbb_config.h"

#ifdef SCBB_OLED_096_SPI_ENABLED

#ifdef SCBB_OLED_096_SPI_SPI_HEADER
#include SCBB_OLED_096_SPI_SPI_HEADER
#endif
#ifdef SCBB_OLED_096_SPI_GPIO_HEADER
#include SCBB_OLED_096_SPI_GPIO_HEADER
#endif
#ifdef SCBB_OLED_096_SPI_DELAY_HEADER
#include SCBB_OLED_096_SPI_DELAY_HEADER
#endif

#include <stdint.h>
#include <stdbool.h>

#define AXK_OLED_096_SPI_W  128  /**< 显示宽度（像素） */
#define AXK_OLED_096_SPI_HEIGHT  64   /**< 显示高度（像素） */

#ifndef AXK_OLED_096_SPI_PIN_CS
#define AXK_OLED_096_SPI_PIN_CS   10  /**< 片选脚（低有效） */
#endif
#ifndef AXK_OLED_096_SPI_PIN_DC
#define AXK_OLED_096_SPI_PIN_DC   1   /**< 数据/命令选择脚 */
#endif
#ifndef AXK_OLED_096_SPI_PIN_RST
#define AXK_OLED_096_SPI_PIN_RST  18  /**< 复位脚 */
#endif
#ifndef AXK_OLED_096_SPI_SPI
#define AXK_OLED_096_SPI_SPI  "spi0" /**< SPI 设备名 */
#endif
#ifndef AXK_OLED_096_SPI_FREQ
#define AXK_OLED_096_SPI_FREQ 4000000u /**< SPI 时钟频率（Hz） */
#endif

/**
 * @brief SSD1306 OLED 运行时配置
 *
 * 一块板挂多个 OLED 时，分别填不同的 pin_cs 后各调一次 init_cfg 即可。
 * spi_dev 必须是常量字符串（如 "spi0"），结构体按值拷贝。
 */
typedef struct {
    uint16_t    pin_cs;     /**< 片选脚（axk 逻辑引脚号） */
    uint16_t    pin_dc;     /**< 数据/命令选择脚 */
    uint16_t    pin_rst;    /**< 复位脚 */
    const char *spi_dev;    /**< SPI 设备名，"spi0" / "spi1" */
    uint32_t    spi_freq;   /**< SPI 时钟频率（Hz），0 用默认值 */
} axk_oled_096_spi_cfg_t;

/**
 * @brief 用运行时配置初始化 SSD1306 OLED
 *
 * @param[in]  cfg  配置结构体指针，不可为 NULL
 * @return     int  0: 成功，-1: 参数错误
 */
int  axk_oled_096_spi_init_cfg(const axk_oled_096_spi_cfg_t *cfg);

/**
 * @brief 用编译期默认配置初始化 SSD1306 OLED 并发送初始化序列
 *
 * @return int  0: 成功
 */
int  axk_oled_096_spi_init(void);

/**
 * @brief 将帧缓冲清为纯色
 *
 * @param[in]  color  0: 全黑，1: 全白
 * @note  调用后需执行 axk_oled_096_spi_flush() 刷新显示
 */
void axk_oled_096_spi_clear(uint8_t color);

/**
 * @brief 在帧缓冲中设置或清除单个像素
 *
 * @param[in]  x   X 坐标（0–127）
 * @param[in]  y   Y 坐标（0–63）
 * @param[in]  on  1: 点亮，0: 熄灭
 */
void axk_oled_096_spi_set_pixel(int x, int y, int on);

/**
 * @brief 将帧缓冲通过 SPI 刷新到 OLED 显示屏
 */
void axk_oled_096_spi_flush(void);

#endif /* SCBB_OLED_096_SPI_ENABLED */
#endif /* AXK_OLED_096_SPI_H */
