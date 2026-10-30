/**
 * @file axk_st7789v_lcd.c
 * @brief ST7789V LCD — 纯 ACLL 跨平台实现（SPI + GPIO + Delay）。
 * @author xemowo (xemowo@qq.com)
 * @version 0.2.0
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 */
#include <stddef.h>

#include "axk_st7789v_lcd.h"

#ifdef SCBB_ST7789V_LCD_ENABLED

#define AXK_ST7789V_SPI_INIT(n,c)     AXK_ST7789V_LCD_SPI_ACLL(init, n, c)
#define AXK_ST7789V_SEND8(n,b)        AXK_ST7789V_LCD_SPI_ACLL(send8, n, b)
#define AXK_ST7789V_SEND16(n,d,cnt)   AXK_ST7789V_LCD_SPI_ACLL(send16, n, d, cnt)
#define AXK_ST7789V_GPIO_INIT(p,m)    AXK_ST7789V_LCD_GPIO_ACLL(init, p, m)
#define AXK_ST7789V_GPIO_SET(p)       AXK_ST7789V_LCD_GPIO_ACLL(set, p)
#define AXK_ST7789V_GPIO_RESET(p)     AXK_ST7789V_LCD_GPIO_ACLL(reset, p)

/** @brief 默认配置（来自编译期宏，可被 axk_st7789v_lcd_init_cfg() 覆盖） */
static const axk_st7789v_lcd_cfg_t s_default_cfg = {
    .pin_cs                = AXK_ST7789V_LCD_PIN_CS,
    .pin_dc                = AXK_ST7789V_LCD_PIN_DC,
    .pin_rst               = AXK_ST7789V_LCD_PIN_RST,
    .pin_blk               = AXK_ST7789V_LCD_PIN_BLK,
    .width                 = AXK_ST7789V_LCD_WIDTH,
    .height                = AXK_ST7789V_LCD_HEIGHT,
    .offset_x              = AXK_ST7789V_LCD_OFFSET_X,
    .offset_y              = AXK_ST7789V_LCD_OFFSET_Y,
    .spi_dev               = AXK_ST7789V_LCD_SPI_DEV,
    .spi_freq              = AXK_ST7789V_LCD_SPI_FREQ,
    .backlight_active_high = AXK_ST7789V_LCD_BL_ACTIVE_HIGH,
};

/** @brief 当前生效配置 */
static axk_st7789v_lcd_cfg_t s_cfg = {
    .pin_cs                = AXK_ST7789V_LCD_PIN_CS,
    .pin_dc                = AXK_ST7789V_LCD_PIN_DC,
    .pin_rst               = AXK_ST7789V_LCD_PIN_RST,
    .pin_blk               = AXK_ST7789V_LCD_PIN_BLK,
    .width                 = AXK_ST7789V_LCD_WIDTH,
    .height                = AXK_ST7789V_LCD_HEIGHT,
    .offset_x              = AXK_ST7789V_LCD_OFFSET_X,
    .offset_y              = AXK_ST7789V_LCD_OFFSET_Y,
    .spi_dev               = AXK_ST7789V_LCD_SPI_DEV,
    .spi_freq              = AXK_ST7789V_LCD_SPI_FREQ,
    .backlight_active_high = AXK_ST7789V_LCD_BL_ACTIVE_HIGH,
};

static bool s_st7789v_inited;

/**
 * @brief 发送一个命令字节
 *
 * @param[in]  cmd  命令字节
 */
static void axk_st7789v_cmd(uint8_t cmd) {
    AXK_ST7789V_GPIO_RESET(s_cfg.pin_dc);
    AXK_ST7789V_GPIO_RESET(s_cfg.pin_cs);
    AXK_ST7789V_SEND8(s_cfg.spi_dev, cmd);
    AXK_ST7789V_GPIO_SET(s_cfg.pin_cs);
}

/**
 * @brief 发送命令字节 + 参数（片选全程拉低）
 *
 * @param[in]  cmd   命令字节
 * @param[in]  data  参数缓冲区，可为 NULL
 * @param[in]  len   参数字节数
 */
static void axk_st7789v_cmd_para(uint8_t cmd, const uint8_t *data, uint16_t len) {
    AXK_ST7789V_GPIO_RESET(s_cfg.pin_dc);
    AXK_ST7789V_GPIO_RESET(s_cfg.pin_cs);
    AXK_ST7789V_SEND8(s_cfg.spi_dev, cmd);
    if (data != NULL && len > 0) {
        AXK_ST7789V_GPIO_SET(s_cfg.pin_dc);
        for (uint16_t i = 0; i < len; i++) {
            AXK_ST7789V_SEND8(s_cfg.spi_dev, data[i]);
        }
    }
    AXK_ST7789V_GPIO_SET(s_cfg.pin_cs);
}

/**
 * @brief 发送面板初始化序列
 */
static void axk_st7789v_init_sequence(void) {
    static const uint8_t b2[] = { 0x0C, 0x0C, 0x00, 0x33, 0x33 };
    static const uint8_t b7[] = { 0x35 };
    static const uint8_t bb[] = { 0x32 };
    static const uint8_t c2[] = { 0x01 };
    static const uint8_t c3[] = { 0x15 };
    static const uint8_t c4[] = { 0x20 };
    static const uint8_t c6[] = { 0x0F };
    static const uint8_t d0[] = { 0xA4, 0xA1 };
    static const uint8_t e0[] = { 0xD0, 0x0A, 0x10, 0x0C, 0x0C, 0x18, 0x35,
                                  0x43, 0x4D, 0x39, 0x13, 0x13, 0x2D, 0x34 };
    static const uint8_t e1[] = { 0xD0, 0x05, 0x0B, 0x06, 0x05, 0x02, 0x35,
                                  0x43, 0x4D, 0x16, 0x15, 0x15, 0x2E, 0x32 };
    static const uint8_t pixfmt = 0x55;   /* RGB565 */
    static const uint8_t madctl = 0x00;   /* 竖屏、RGB 顺序 */

    axk_st7789v_cmd(0x01);                /* 软复位 */
    AXK_ST7789V_LCD_DELAY_MS(10);
    axk_st7789v_cmd(0x11);                /* 退出睡眠 */
    AXK_ST7789V_LCD_DELAY_MS(120);

    axk_st7789v_cmd_para(0xB2, b2, sizeof(b2));
    axk_st7789v_cmd_para(0xB7, b7, sizeof(b7));
    axk_st7789v_cmd_para(0xBB, bb, sizeof(bb));
    axk_st7789v_cmd_para(0xC2, c2, sizeof(c2));
    axk_st7789v_cmd_para(0xC3, c3, sizeof(c3));
    axk_st7789v_cmd_para(0xC4, c4, sizeof(c4));
    axk_st7789v_cmd_para(0xC6, c6, sizeof(c6));
    axk_st7789v_cmd_para(0xD0, d0, sizeof(d0));
    axk_st7789v_cmd_para(0xE0, e0, sizeof(e0));
    axk_st7789v_cmd_para(0xE1, e1, sizeof(e1));
    axk_st7789v_cmd_para(0x3A, &pixfmt, 1);
    axk_st7789v_cmd(0x21);                /* IPS 屏通常需要反显 */
    axk_st7789v_cmd_para(0x36, &madctl, 1);
    axk_st7789v_cmd(0x29);                /* 开显示 */
}

/**
 * @brief 用运行时配置初始化 ST7789V LCD
 *
 * @param[in]  cfg  配置结构体指针
 * @return     int  0: 成功，-1: 参数错误，-2: SPI 初始化失败
 */
int axk_st7789v_lcd_init_cfg(const axk_st7789v_lcd_cfg_t *cfg) {
    bsp_spi_cfg_t spi_cfg;

    if (cfg == NULL || cfg->spi_dev == NULL || cfg->width == 0 || cfg->height == 0) {
        return -1;
    }
    s_cfg = *cfg;
    s_st7789v_inited = false;

    /* 控制脚初始化，片选/DC 先拉高；背光先关 */
    AXK_ST7789V_GPIO_INIT(s_cfg.pin_cs, 1);
    AXK_ST7789V_GPIO_SET(s_cfg.pin_cs);
    AXK_ST7789V_GPIO_INIT(s_cfg.pin_dc, 1);
    AXK_ST7789V_GPIO_SET(s_cfg.pin_dc);
    AXK_ST7789V_GPIO_INIT(s_cfg.pin_rst, 1);
    if (s_cfg.pin_blk != AXK_ST7789V_LCD_PIN_NONE) {
        AXK_ST7789V_GPIO_INIT(s_cfg.pin_blk, 1);
        axk_st7789v_lcd_backlight(false);
    }

    /* 硬件复位：低 20ms，释放后等 120ms */
    AXK_ST7789V_GPIO_RESET(s_cfg.pin_rst);
    AXK_ST7789V_LCD_DELAY_MS(20);
    AXK_ST7789V_GPIO_SET(s_cfg.pin_rst);
    AXK_ST7789V_LCD_DELAY_MS(120);

    /* SPI：模式 0，8 位，MSB 先行 */
    spi_cfg.freq = (s_cfg.spi_freq != 0u) ? s_cfg.spi_freq : 1000000u;
    spi_cfg.mode = 0;
    spi_cfg.data_width = 8;
    if (AXK_ST7789V_SPI_INIT(s_cfg.spi_dev, &spi_cfg) != 0) {
        return -2;
    }

    axk_st7789v_init_sequence();

    s_st7789v_inited = true;
    axk_st7789v_lcd_clear(0x0000);
    axk_st7789v_lcd_backlight(true);
    return 0;
}

/**
 * @brief 用编译期默认配置初始化 ST7789V LCD
 *
 * @return int  0: 成功，错误码同 axk_st7789v_lcd_init_cfg()
 */
int axk_st7789v_lcd_init(void) {
    return axk_st7789v_lcd_init_cfg(&s_default_cfg);
}

/**
 * @brief 开关 LCD 背光
 *
 * @param[in]  on  true: 开，false: 关
 */
void axk_st7789v_lcd_backlight(bool on) {
    bool level;

    if (s_cfg.pin_blk == AXK_ST7789V_LCD_PIN_NONE) {
        return;                 /* 无背光控制脚：背光常亮，忽略开关请求 */
    }

    level = on ? (s_cfg.backlight_active_high != 0u)
               : (s_cfg.backlight_active_high == 0u);

    if (level) {
        AXK_ST7789V_GPIO_SET(s_cfg.pin_blk);
    } else {
        AXK_ST7789V_GPIO_RESET(s_cfg.pin_blk);
    }
}

/**
 * @brief 设置活动绘制窗口（CASET + RASET）
 *
 * @param[in]  x1  起始 X
 * @param[in]  y1  起始 Y
 * @param[in]  x2  结束 X
 * @param[in]  y2  结束 Y
 */
void axk_st7789v_lcd_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    uint8_t p[4];
    uint16_t xs, xe, ys, ye;

    if (!s_st7789v_inited) {
        return;
    }
    xs = (uint16_t)(x1 + s_cfg.offset_x);
    xe = (uint16_t)(x2 + s_cfg.offset_x);
    ys = (uint16_t)(y1 + s_cfg.offset_y);
    ye = (uint16_t)(y2 + s_cfg.offset_y);

    p[0] = (uint8_t)(xs >> 8);
    p[1] = (uint8_t)(xs & 0xFF);
    p[2] = (uint8_t)(xe >> 8);
    p[3] = (uint8_t)(xe & 0xFF);
    axk_st7789v_cmd_para(0x2A, p, 4);

    p[0] = (uint8_t)(ys >> 8);
    p[1] = (uint8_t)(ys & 0xFF);
    p[2] = (uint8_t)(ye >> 8);
    p[3] = (uint8_t)(ye & 0xFF);
    axk_st7789v_cmd_para(0x2B, p, 4);
}

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
                                  uint16_t *pixels) {
    uint32_t count;

    if (!s_st7789v_inited || pixels == NULL || x1 > x2 || y1 > y2) {
        return;
    }
    count = (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1);

    axk_st7789v_lcd_set_window(x1, y1, x2, y2);

    /* 0x2C 之后连续写入 count 个 RGB565 像素，片选全程拉低 */
    AXK_ST7789V_GPIO_RESET(s_cfg.pin_dc);
    AXK_ST7789V_GPIO_RESET(s_cfg.pin_cs);
    AXK_ST7789V_SEND8(s_cfg.spi_dev, 0x2C);
    AXK_ST7789V_GPIO_SET(s_cfg.pin_dc);
    AXK_ST7789V_SEND16(s_cfg.spi_dev, pixels, count);
    AXK_ST7789V_GPIO_SET(s_cfg.pin_cs);
}

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
                          uint16_t color) {
    uint16_t buf[64];
    uint32_t count;

    if (!s_st7789v_inited || x1 > x2 || y1 > y2) {
        return;
    }
    count = (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1);

    for (unsigned int i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
        buf[i] = color;
    }

    axk_st7789v_lcd_set_window(x1, y1, x2, y2);
    AXK_ST7789V_GPIO_RESET(s_cfg.pin_dc);
    AXK_ST7789V_GPIO_RESET(s_cfg.pin_cs);
    AXK_ST7789V_SEND8(s_cfg.spi_dev, 0x2C);
    AXK_ST7789V_GPIO_SET(s_cfg.pin_dc);
    while (count > 0u) {
        uint32_t n = (count > sizeof(buf) / sizeof(buf[0]))
                         ? (uint32_t)(sizeof(buf) / sizeof(buf[0])) : count;
        AXK_ST7789V_SEND16(s_cfg.spi_dev, buf, n);
        count -= n;
    }
    AXK_ST7789V_GPIO_SET(s_cfg.pin_cs);
}

/**
 * @brief 清屏为单一颜色
 *
 * @param[in]  color  RGB565 填充颜色
 */
void axk_st7789v_lcd_clear(uint16_t color) {
    if (!s_st7789v_inited) {
        return;
    }
    axk_st7789v_lcd_fill(0, 0, (uint16_t)(s_cfg.width - 1u),
                         (uint16_t)(s_cfg.height - 1u), color);
}

#endif /* SCBB_ST7789V_LCD_ENABLED */
