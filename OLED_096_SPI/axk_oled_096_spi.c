/**
 * @file axk_oled_096_spi.c
 * @brief SSD1306 0.96" OLED 128x64 SPI — 纯 ACLL 跨平台实现。
 * @author xemowo (xemowo@qq.com)
 * @version 0.1.1
 * @date 2026-07-31
 *
 * @copyright Copyright (c) 2026
 */
#include <string.h>

#include "axk_oled_096_spi.h"

#ifdef SCBB_OLED_096_SPI_ENABLED

#define AXK_OLED_096_SPI_SPI_INIT(n,c)  AXK_OLED_096_SPI_SPI_ACLL(init,n,c)
#define AXK_OLED_096_SPI_SEND8(n,b)  AXK_OLED_096_SPI_SPI_ACLL(send8,n,b)
#define AXK_OLED_096_SPI_GPIO_INIT(p,m)  AXK_OLED_096_SPI_GPIO_ACLL(init,p,m)
#define AXK_OLED_096_SPI_GPIO_SET(p)    AXK_OLED_096_SPI_GPIO_ACLL(set,p)
#define AXK_OLED_096_SPI_GPIO_RESET(p)    AXK_OLED_096_SPI_GPIO_ACLL(reset,p)

/** @brief 默认配置（来自编译期宏，可被 axk_oled_096_spi_init_cfg() 覆盖） */
static const axk_oled_096_spi_cfg_t s_default_cfg = {
    .pin_cs   = AXK_OLED_096_SPI_PIN_CS,
    .pin_dc   = AXK_OLED_096_SPI_PIN_DC,
    .pin_rst  = AXK_OLED_096_SPI_PIN_RST,
    .spi_dev  = AXK_OLED_096_SPI_SPI,
    .spi_freq = AXK_OLED_096_SPI_FREQ,
};

/** @brief 当前生效配置 */
static axk_oled_096_spi_cfg_t s_cfg = {
    .pin_cs   = AXK_OLED_096_SPI_PIN_CS,
    .pin_dc   = AXK_OLED_096_SPI_PIN_DC,
    .pin_rst  = AXK_OLED_096_SPI_PIN_RST,
    .spi_dev  = AXK_OLED_096_SPI_SPI,
    .spi_freq = AXK_OLED_096_SPI_FREQ,
};

/**
 * @brief 片选拉低
 */
static void axk_oled_cs_low(void) {
    AXK_OLED_096_SPI_GPIO_RESET(s_cfg.pin_cs);
}

/**
 * @brief 片选拉高
 */
static void axk_oled_cs_high(void) {
    AXK_OLED_096_SPI_GPIO_SET(s_cfg.pin_cs);
}

/**
 * @brief 数据/命令脚拉低（命令模式）
 */
static void axk_oled_dc_low(void) {
    AXK_OLED_096_SPI_GPIO_RESET(s_cfg.pin_dc);
}

/**
 * @brief 数据/命令脚拉高（数据模式）
 */
static void axk_oled_dc_high(void) {
    AXK_OLED_096_SPI_GPIO_SET(s_cfg.pin_dc);
}

/**
 * @brief 向 SSD1306 发送命令字节
 *
 * @param[in]  c  命令字节值
 */
static void axk_oled_cmd(uint8_t c) {
    axk_oled_dc_low();
    axk_oled_cs_low();
    AXK_OLED_096_SPI_SEND8(s_cfg.spi_dev, c);
    axk_oled_cs_high();
}

/**
 * @brief 向 SSD1306 发送数据字节
 *
 * @param[in]  d  数据字节值
 */
static void axk_oled_dat(uint8_t d) {
    axk_oled_dc_high();
    axk_oled_cs_low();
    AXK_OLED_096_SPI_SEND8(s_cfg.spi_dev, d);
    axk_oled_cs_high();
}

static uint8_t s_axk_oled_fb[128][8];

/**
 * @brief 用运行时配置初始化 SSD1306 OLED：GPIO、SPI、初始化序列、清屏
 *
 * @param[in]  cfg  配置结构体指针，不可为 NULL
 * @return     int  0: 成功，-1: 参数错误
 */
int axk_oled_096_spi_init_cfg(const axk_oled_096_spi_cfg_t *cfg) {
    bsp_spi_cfg_t c;

    if (cfg == NULL || cfg->spi_dev == NULL) {
        return -1;
    }
    s_cfg = *cfg;

    AXK_OLED_096_SPI_GPIO_INIT(s_cfg.pin_cs, 1);
    axk_oled_cs_high();
    AXK_OLED_096_SPI_GPIO_INIT(s_cfg.pin_dc, 1);
    axk_oled_dc_high();
    AXK_OLED_096_SPI_GPIO_INIT(s_cfg.pin_rst, 1);
    AXK_OLED_096_SPI_GPIO_RESET(s_cfg.pin_rst);
    AXK_OLED_096_SPI_DELAY_MS(10);
    AXK_OLED_096_SPI_GPIO_SET(s_cfg.pin_rst);
    AXK_OLED_096_SPI_DELAY_MS(100);

    c.freq = (s_cfg.spi_freq != 0u) ? s_cfg.spi_freq : AXK_OLED_096_SPI_FREQ;
    c.mode = 3;
    c.data_width = 8;
    AXK_OLED_096_SPI_SPI_INIT(s_cfg.spi_dev, &c);

    uint8_t init[] = {
        0xAE,        /* 关闭显示 */
        0xD5,0x80,   /* 时钟分频 */
        0xA8,0x3F,   /* 多路复用比 64 */
        0xD3,0x00,   /* 显示偏移 */
        0x40,        /* 起始行 */
        0x8D,0x14,   /* 电荷泵 */
        0x20,0x00,   /* 水平寻址 */
        0xA1,        /* 段重映射 */
        0xC8,        /* COM 扫描方向 */
        0xDA,0x12,   /* COM 引脚配置 */
        0x81,0xCF,   /* 对比度 */
        0xD9,0xF1,   /* 预充电 */
        0xDB,0x40,   /* VCOM 检测 */
        0xA4,        /* 显示恢复 */
        0xA6,        /* 正常显示 */
        0xAF,        /* 开启显示 */
    };
    for (unsigned int i = 0; i < sizeof(init); i++) {
        axk_oled_cmd(init[i]);
    }
    axk_oled_096_spi_clear(0);
    axk_oled_096_spi_flush();
    return 0;
}

/**
 * @brief 用编译期默认配置初始化 SSD1306 OLED
 *
 * @return int  0: 成功，错误码同 axk_oled_096_spi_init_cfg()
 */
int axk_oled_096_spi_init(void) {
    return axk_oled_096_spi_init_cfg(&s_default_cfg);
}

/**
 * @brief 将帧缓冲清为空色
 *
 * @param[in]  color  0: 全黑，1: 全白
 */
void axk_oled_096_spi_clear(uint8_t color) {
    memset(s_axk_oled_fb, color ? 0xFF : 0x00, sizeof(s_axk_oled_fb));
}

/**
 * @brief 在帧缓冲中设置或清除单个像素
 *
 * @param[in]  x   X 坐标（0–127）
 * @param[in]  y   Y 坐标（0–63）
 * @param[in]  on  1: 点亮，0: 熄灭
 */
void axk_oled_096_spi_set_pixel(int x, int y, int on) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) {
        return;
    }
    if (on) {
        s_axk_oled_fb[x][y / 8] |= (1u << (y % 8));
    } else {
        s_axk_oled_fb[x][y / 8] &= ~(1u << (y % 8));
    }
}

/**
 * @brief 通过 SPI 将 8 页全部刷新到 OLED 显示屏
 */
void axk_oled_096_spi_flush(void) {
    for (int page = 0; page < 8; page++) {
        axk_oled_cmd(0xB0 + page);
        axk_oled_cmd(0x00);
        axk_oled_cmd(0x10);
        for (int x = 0; x < 128; x++) {
            axk_oled_dat(s_axk_oled_fb[x][page]);
        }
    }
}
#endif /* SCBB_OLED_096_SPI_ENABLED */
