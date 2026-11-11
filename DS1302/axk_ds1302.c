/**
 * @file axk_ds1302.c
 * @brief DS1302 RTC — 纯 ACLL GPIO 位操作实现。
 * @author xemowo (xemowo@qq.com)
 * @version 0.1.2
 * @date 2026-11-10
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_ds1302.h"

#ifdef SCBB_DS1302_ENABLED

#define AXK_DS1302_GPIO_INIT(p,m) AXK_DS1302_GPIO_ACLL(init,p,m)
#define AXK_DS1302_GPIO_SET(p)   AXK_DS1302_GPIO_ACLL(set,p)
#define AXK_DS1302_GPIO_RESET(p)   AXK_DS1302_GPIO_ACLL(reset,p)
#define AXK_DS1302_GPIO_READ(p)  AXK_DS1302_GPIO_ACLL(read,p)

static uint16_t s_axk_ds1302_pin_ce   = AXK_DS1302_PIN_CE;
static uint16_t s_axk_ds1302_pin_sclk = AXK_DS1302_PIN_SCLK;
static uint16_t s_axk_ds1302_pin_io   = AXK_DS1302_PIN_IO;

/**
 * @brief 数据脚切换为输出模式
 */
static void axk_ds1302_io_out(void) {
    AXK_DS1302_GPIO_INIT(s_axk_ds1302_pin_io, 1);
}

/**
 * @brief 数据脚切换为输入模式
 */
static void axk_ds1302_io_in(void) {
    AXK_DS1302_GPIO_INIT(s_axk_ds1302_pin_io, 0);
}

/**
 * @brief 产生一个时钟脉冲（SCLK 拉高-延时-拉低-延时）
 *
 * DS1302 在 SCLK 上升沿锁存输入数据，在下降沿输出下一位数据。
 * 下降沿后留 1us，保证主机下一次读取时数据已经稳定。
 */
static void axk_ds1302_sclk(void) {
    AXK_DS1302_GPIO_SET(s_axk_ds1302_pin_sclk);
    AXK_DS1302_DELAY_US(1);
    AXK_DS1302_GPIO_RESET(s_axk_ds1302_pin_sclk);
    AXK_DS1302_DELAY_US(1);
}

/**
 * @brief 通过 3 线接口写一个字节（LSB 优先）
 *
 * 数据在 SCLK 上升沿被 DS1302 锁存，所以先把 IO 设好再拉高 SCLK。
 * 进入前会把 IO 自动切成输出，写完保持输出状态。
 *
 * @param[in] d 要写入的字节
 */
static void axk_ds1302_write_byte(uint8_t d) {
    axk_ds1302_io_out();
    for (int i = 0; i < 8; i++) {
        if ((d & 1) != 0) {
            AXK_DS1302_GPIO_SET(s_axk_ds1302_pin_io);
        } else {
            AXK_DS1302_GPIO_RESET(s_axk_ds1302_pin_io);
        }
        d >>= 1;
        AXK_DS1302_DELAY_US(1);     /* 数据建立时间 */
        axk_ds1302_sclk();
    }
}

/**
 * @brief 通过 3 线接口读一个字节（LSB 优先）
 *
 * 关键时序：DS1302 在 SCLK 下降沿输出下一位，数据在 SCLK 为低
 * 期间有效。命令字最后一个下降沿已经把本字节的 bit0 送出来了，
 * 所以要先读当前位、再发时钟脉冲翻到下一位。
 *
 * 原实现是“先发时钟再读”，会把 bit1 当成 bit0，整串时间错一位，
 * 读出来就是乱码或 RTC ERR。
 *
 * @return uint8_t 读到的字节
 */
static uint8_t axk_ds1302_read_byte(void) {
    uint8_t r = 0;
    axk_ds1302_io_in();
    AXK_DS1302_DELAY_US(1);         /* 等 DS1302 把 bit0 驱动稳定 */
    for (int i = 0; i < 8; i++) {
        if (AXK_DS1302_GPIO_READ(s_axk_ds1302_pin_io)) {
            r |= (1u << i);
        }
        axk_ds1302_sclk();          /* 下降沿输出下一位 */
    }
    return r;
}

/**
 * @brief 用运行时指定的 3 个引脚初始化 DS1302
 *
 * @param pin_ce   使能脚
 * @param pin_sclk 时钟脚
 * @param pin_io   数据 I/O 脚
 * @return int     0: 成功
 */
int axk_ds1302_init_pins(uint16_t pin_ce, uint16_t pin_sclk, uint16_t pin_io) {
    s_axk_ds1302_pin_ce   = pin_ce;
    s_axk_ds1302_pin_sclk = pin_sclk;
    s_axk_ds1302_pin_io   = pin_io;

    AXK_DS1302_GPIO_INIT(s_axk_ds1302_pin_ce, 1); AXK_DS1302_GPIO_RESET(s_axk_ds1302_pin_ce);
    AXK_DS1302_GPIO_INIT(s_axk_ds1302_pin_sclk, 1); AXK_DS1302_GPIO_RESET(s_axk_ds1302_pin_sclk);
    axk_ds1302_io_out(); AXK_DS1302_GPIO_SET(s_axk_ds1302_pin_io);
    return 0;
}

/**
 * @brief 用编译期 AXK_DS1302_PIN_* 初始化 DS1302
 *
 * @return int  0: 成功
 */
int axk_ds1302_init(void) {
    return axk_ds1302_init_pins(AXK_DS1302_PIN_CE, AXK_DS1302_PIN_SCLK, AXK_DS1302_PIN_IO);
}

/**
 * @brief BCD 码转十进制
 *
 * @param[in]  b       BCD 码
 * @return     uint8_t 十进制值
 */
static uint8_t axk_ds1302_bcd2dec(uint8_t b) {
    return (b >> 4) * 10 + (b & 0x0F);
}

/**
 * @brief 十进制转 BCD 码
 *
 * @param[in]  d       十进制值（0-99）
 * @return     uint8_t BCD 码
 */
static uint8_t axk_ds1302_dec2bcd(uint8_t d) {
    return ((d / 10) << 4) | (d % 10);
}

/**
 * @brief 突发读取 DS1302 当前时间（7 字节）
 *
 * @param[out] *t   时间结构体指针
 * @return     int  0: 成功，-1: 空指针
 */
int axk_ds1302_get_time(axk_ds1302_time_t *t) {
    if (!t) {
        return -1;
    }
    uint8_t raw[7];

    /* 命令字必须由主机驱动 IO；读完保持输出高，下一次访问不用再 init */
    axk_ds1302_io_out();
    AXK_DS1302_GPIO_SET(s_axk_ds1302_pin_ce);
    axk_ds1302_write_byte(0xBF);
    for (int i = 0; i < 7; i++) {
        raw[i] = axk_ds1302_read_byte();
    }
    AXK_DS1302_GPIO_RESET(s_axk_ds1302_pin_ce);
    AXK_DS1302_GPIO_SET(s_axk_ds1302_pin_io);
    axk_ds1302_io_out();

    t->sec  = axk_ds1302_bcd2dec(raw[0] & 0x7F);
    t->min  = axk_ds1302_bcd2dec(raw[1] & 0x7F);
    /* 小时寄存器 bit7=1 表示 12 小时制：bit5 是 AM/PM，bit4:0 是 1-12。
     * 统一换算成 0-23，避免 12 小时制的芯片读出来是 32/40 这类值。 */
    if ((raw[2] & 0x80) != 0) {
        int hour = axk_ds1302_bcd2dec(raw[2] & 0x1F);
        int pm   = (raw[2] & 0x20) != 0;
        if (hour == 12) {
            hour = 0;
        }
        t->hour = hour + (pm ? 12 : 0);
    } else {
        t->hour = axk_ds1302_bcd2dec(raw[2] & 0x3F);
    }
    t->day  = axk_ds1302_bcd2dec(raw[3] & 0x3F);
    t->mon  = axk_ds1302_bcd2dec(raw[4] & 0x1F);
    t->week = raw[5] & 0x07;        /* 周寄存器是 1-7 二进制，不是 BCD */
    t->year = axk_ds1302_bcd2dec(raw[6]);
    return 0;
}

/**
 * @brief 突发写入 DS1302 时间（先关闭写保护）
 *
 * @param[in]  *t   时间结构体指针
 * @return     int  0: 成功，-1: 空指针
 */
int axk_ds1302_set_time(const axk_ds1302_time_t *t) {
    if (!t) {
        return -1;
    }
    /* 时间字段范围校验（BCD 编码前） */
    if (t->sec < 0 || t->sec > 59 || t->min < 0 || t->min > 59 ||
        t->hour < 0 || t->hour > 23 || t->day < 1 || t->day > 31 ||
        t->mon < 1 || t->mon > 12 || t->week < 1 || t->week > 7 ||
        t->year < 0 || t->year > 99) {
        return -2;
    }
    axk_ds1302_io_out();
    AXK_DS1302_GPIO_SET(s_axk_ds1302_pin_ce);
    axk_ds1302_write_byte(0x8E);        /* 关闭写保护 */
    axk_ds1302_write_byte(0x00);
    AXK_DS1302_GPIO_RESET(s_axk_ds1302_pin_ce);
    AXK_DS1302_GPIO_SET(s_axk_ds1302_pin_ce);
    axk_ds1302_write_byte(0xBE);        /* 突发写 7 字节 */
    uint8_t d[] = { axk_ds1302_dec2bcd(t->sec), axk_ds1302_dec2bcd(t->min), axk_ds1302_dec2bcd(t->hour),
                    axk_ds1302_dec2bcd(t->day), axk_ds1302_dec2bcd(t->mon), axk_ds1302_dec2bcd(t->week), axk_ds1302_dec2bcd(t->year) };
    for (int i = 0; i < 7; i++) {
        axk_ds1302_write_byte(d[i]);
    }
    AXK_DS1302_GPIO_RESET(s_axk_ds1302_pin_ce);
    AXK_DS1302_GPIO_SET(s_axk_ds1302_pin_io);  /* 空闲保持输出高 */
    return 0;
}
#endif /* SCBB_DS1302_ENABLED */
