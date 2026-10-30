/**
 * @file axk_st7789v_lcd.h
 * @brief ST7789V 驱动芯片通用 LCD 模块（SPI，纯 ACLL 跨平台）。
 *
 * 适用于所有 ST7789V 驱动屏幕：1.47"、1.69"、1.9"、2.0" 等。
 * 分辨率、偏移量、片选/数据/复位/背光引脚都可以在运行时通过
 * axk_st7789v_lcd_cfg_t 指定，不需要改 BSP 或工程编译选项。
 *
 * 注意：SPI 的 SCL/SDA 属于总线引脚，不归模块管，由所选 BSP 统一配置。
 * axk 平台见 BSP/axk/axk_bsp_config.h 里的 AXK_BSP_SPI0_* / AXK_BSP_SPI1_*；
 * 换总线引脚请改 BSP 配置，不是改这里的结构体。
 *
 * 默认模块引脚（AI-M6x / BL616，可通过 #define 覆盖）：
 *   CS=IO10, DC=IO1, RST=IO18, BLK=IO20
 *
 * @author xemowo (xemowo@qq.com)
 * @version 0.2.0
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_ST7789V_LCD_H
#define AXK_ST7789V_LCD_H

#include "scbb_config.h"

#ifdef SCBB_ST7789V_LCD_ENABLED

#ifdef SCBB_ST7789V_LCD_SPI_HEADER
#include SCBB_ST7789V_LCD_SPI_HEADER
#endif
#ifdef SCBB_ST7789V_LCD_GPIO_HEADER
#include SCBB_ST7789V_LCD_GPIO_HEADER
#endif
#ifdef SCBB_ST7789V_LCD_DELAY_HEADER
#include SCBB_ST7789V_LCD_DELAY_HEADER
#endif

#include <stdint.h>
#include <stdbool.h>

/** @brief 显示宽度（像素，默认 1.47" 屏） */
#ifndef AXK_ST7789V_LCD_WIDTH
#define AXK_ST7789V_LCD_WIDTH   172
#endif
/** @brief 显示高度（像素） */
#ifndef AXK_ST7789V_LCD_HEIGHT
#define AXK_ST7789V_LCD_HEIGHT  320
#endif
/** @brief X 偏移：172px 居中于 240px GRAM */
#ifndef AXK_ST7789V_LCD_OFFSET_X
#define AXK_ST7789V_LCD_OFFSET_X 34
#endif
/** @brief Y 偏移 */
#ifndef AXK_ST7789V_LCD_OFFSET_Y
#define AXK_ST7789V_LCD_OFFSET_Y 0
#endif

#ifndef AXK_ST7789V_LCD_PIN_CS
#define AXK_ST7789V_LCD_PIN_CS    10    /**< 片选脚（低有效） */
#endif
#ifndef AXK_ST7789V_LCD_PIN_DC
#define AXK_ST7789V_LCD_PIN_DC    1     /**< 数据/命令选择脚 */
#endif
#ifndef AXK_ST7789V_LCD_PIN_RST
#define AXK_ST7789V_LCD_PIN_RST   18    /**< 复位脚 */
#endif
#ifndef AXK_ST7789V_LCD_PIN_BLK
#define AXK_ST7789V_LCD_PIN_BLK   20    /**< 背光控制脚 */
#endif

/**
 * @brief pin_blk 取该值时表示“没有背光控制脚”
 *
 * 有些板子（如 AiPi-Clock-Mini）背光直接接 3V3 常亮，没有 BLK 控制脚。
 * 此时 cfg.pin_blk 填 AXK_ST7789V_LCD_PIN_NONE，驱动会跳过背光脚的
 * 初始化和开关，避免误驱动一块板上恰好复用该脚的外设。
 */
#ifndef AXK_ST7789V_LCD_PIN_NONE
#define AXK_ST7789V_LCD_PIN_NONE  0xFFFFu
#endif
#ifndef AXK_ST7789V_LCD_BL_ACTIVE_HIGH
#define AXK_ST7789V_LCD_BL_ACTIVE_HIGH 1 /**< 背光有效电平：1 高有效，0 低有效 */
#endif

#ifndef AXK_ST7789V_LCD_SPI_DEV
#define AXK_ST7789V_LCD_SPI_DEV   "spi0" /**< SPI 设备名（对应 BSP 的引脚组） */
#endif
#ifndef AXK_ST7789V_LCD_SPI_FREQ
#define AXK_ST7789V_LCD_SPI_FREQ  40000000 /**< SPI 时钟频率（Hz） */
#endif

/**
 * @brief ST7789V LCD 运行时配置
 *
 * 在 main() 或任务里填好结构体后调用 axk_st7789v_lcd_init_cfg() 即可，
 * spi_dev 必须是常量字符串（如 "spi0"），结构体按值拷贝，不需要长期持有。
 */
typedef struct {
    uint16_t    pin_cs;                 /**< 片选脚（axk 逻辑引脚号） */
    uint16_t    pin_dc;                 /**< 数据/命令选择脚 */
    uint16_t    pin_rst;                /**< 复位脚 */
    uint16_t    pin_blk;                /**< 背光控制脚 */
    uint16_t    width;                  /**< 显示宽度（像素） */
    uint16_t    height;                 /**< 显示高度（像素） */
    uint16_t    offset_x;               /**< X 偏移（GRAM 居中用） */
    uint16_t    offset_y;               /**< Y 偏移 */
    const char *spi_dev;                /**< SPI 设备名，"spi0" / "spi1" */
    uint32_t    spi_freq;               /**< SPI 时钟频率（Hz） */
    uint8_t     backlight_active_high;  /**< 1: 高电平点亮；0: 低电平点亮 */
} axk_st7789v_lcd_cfg_t;

/**
 * @brief 用运行时配置初始化 ST7789V LCD
 *
 * 执行硬件复位、SPI 初始化、面板初始化序列、清屏，并打开背光。
 *
 * @param[in]  cfg  配置结构体指针，不可为 NULL；width/height/spi_dev 必须有效
 * @return     int  0: 成功，-1: 参数错误，-2: SPI 初始化失败
 */
int  axk_st7789v_lcd_init_cfg(const axk_st7789v_lcd_cfg_t *cfg);

/**
 * @brief 用编译期 AXK_ST7789V_LCD_* 默认配置初始化
 *
 * @return int  0: 成功，错误码同 axk_st7789v_lcd_init_cfg()
 */
int  axk_st7789v_lcd_init(void);

/**
 * @brief 开关 LCD 背光
 *
 * @param[in]  on  true: 开，false: 关
 */
void axk_st7789v_lcd_backlight(bool on);

/**
 * @brief 设置活动绘制窗口（CASET + RASET）
 *
 * @param[in]  x1  起始 X（含）
 * @param[in]  y1  起始 Y（含）
 * @param[in]  x2  结束 X（含）
 * @param[in]  y2  结束 Y（含）
 */
void axk_st7789v_lcd_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * @brief 向指定窗口绘制像素块（阻塞传输）
 *
 * @param[in]  x1      起始 X
 * @param[in]  y1      起始 Y
 * @param[in]  x2      结束 X
 * @param[in]  y2      结束 Y
 * @param[in]  *pixels RGB565 像素数据
 */
void axk_st7789v_lcd_draw_pixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                                  uint16_t *pixels);

/**
 * @brief 用纯色填充矩形区域
 *
 * @param[in]  x1    起始 X
 * @param[in]  y1    起始 Y
 * @param[in]  x2    结束 X
 * @param[in]  y2    结束 Y
 * @param[in]  color RGB565 填充颜色
 */
void axk_st7789v_lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                          uint16_t color);

/**
 * @brief 清屏为单一颜色
 *
 * @param[in]  color  RGB565 填充颜色
 */
void axk_st7789v_lcd_clear(uint16_t color);

#endif /* SCBB_ST7789V_LCD_ENABLED */
#endif /* AXK_ST7789V_LCD_H */
