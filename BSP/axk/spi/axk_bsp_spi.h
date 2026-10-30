/**
 * @file axk_bsp_spi.h
 * @brief axk BSP SPI 接口（GPIO 软件模拟，无芯片依赖）
 *
 * 与 Ai-M6x BSP 的 bsp_spi_* 接口一致，另补一个 transfer8（LLCC68 需要）。
 * 按设备名选总线："spi0" → 第 0 组引脚，"spi1" → 第 1 组引脚，
 * 引脚在 ../axk_bsp_config.h 里配置。
 *
 * CS 由模块自己用 GPIO 控制，本驱动不碰 CS。
 * MISO 与 MOSI 同脚 = 只写不回读（OLED 就是这样接的）。
 *
 * 说明：软件模拟的实际速率受 GPIO 调用开销限制，freq 只用于插入
 * 半周期延时；freq 高于约 500kHz 时不再插延时，跑多快取决于芯片。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_SPI_H
#define AXK_BSP_SPI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BSP_SPI_CFG_T_DEFINED
#define BSP_SPI_CFG_T_DEFINED
/** @brief SPI 配置结构体（与 Ai-M6x BSP 保持一致） */
typedef struct {
    uint32_t freq;       /**< SPI 时钟频率（Hz） */
    uint8_t  mode;       /**< SPI 模式（0-3，CPOL/CPHA 组合） */
    uint8_t  data_width; /**< 数据宽度（8 或 16，仅作记录） */
} bsp_spi_cfg_t;
#endif

/**
 * @brief 初始化 SPI 总线（主机模式，MSB 先行）
 * @param name 设备名："spi0" 或 "spi1"
 * @param cfg  配置，NULL 返回 -1
 * @return 0 成功，-1 参数错误
 */
int axk_bsp_spi_init(const char *name, bsp_spi_cfg_t *cfg);

/**
 * @brief 初始化 SPI 总线并显式指定引脚（运行时可改，优先于板级表）
 *
 * 适合在 main.c 里按实际接线设置总线脚；模块随后 init 时复用这里
 * 指定的引脚，不会被覆盖。miso 与 mosi 填同一引脚表示“只写不回读”。
 *
 * @param name  设备名："spi0" 或 "spi1"
 * @param sclk  时钟脚（axk 逻辑引脚号）
 * @param mosi  MOSI 数据脚
 * @param miso  MISO 数据脚（无回读时与 mosi 相同）
 * @param cfg   配置，NULL 返回 -1
 * @return 0 成功，-1 参数错误
 */
int axk_bsp_spi_init_ex(const char *name, uint16_t sclk, uint16_t mosi,
                        uint16_t miso, bsp_spi_cfg_t *cfg);

/** @brief 发送 1 字节（忽略读回） */
void axk_bsp_spi_send8(const char *name, uint8_t data);

/** @brief 发送 n 个 16 位数据（高字节在前） */
void axk_bsp_spi_send16(const char *name, const uint16_t *data, uint32_t n);

/** @brief 全双工交换 1 字节（发送的同时读回） */
uint8_t axk_bsp_spi_transfer8(const char *name, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* AXK_BSP_SPI_H */
