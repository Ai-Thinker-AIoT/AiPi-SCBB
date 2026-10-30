/**
 * @file axk_bsp_i2s.h
 * @brief axk BSP —— I2S（open SDK aiio_hal_i2s 适配）
 *
 * 对应中间件 API：aiio_hal_i2s_init() / aiio_hal_i2s_deinit()
 *
 * 芯片支持：目前仅 BL616 / BL616CL / BL618 实现，其余芯片返回
 * AXK_BSP_ERR_UNSUPPORTED。
 *
 * 已知限制：
 *   · 中间件只提供初始化/反初始化，没有收发数据接口
 *   · BL 实现在 init 里固定 16kHz / 16bit / 双声道，并在内部启动 DMA
 *     （数据落在驱动自己的 rx_buffer 里）
 *   · MCLK 引脚会被配置成 CLKOUT，且会改 I2S 时钟源
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_I2S_H
#define AXK_BSP_I2S_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief I2S 端口（与 aiio_i2s_enum_t 数值一致） */
typedef enum {
    AXK_BSP_I2S_PORT0 = 0,
    AXK_BSP_I2S_PORT1 = 1,
    AXK_BSP_I2S_PORT2 = 2,
} axk_bsp_i2s_port_t;

typedef struct {
    uint8_t  port;   /*!< axk_bsp_i2s_port_t */
    uint16_t fs;     /*!< 帧同步 LRCK，axk 编码引脚 */
    uint16_t di;     /*!< 数据输入 */
    uint16_t do_;    /*!< 数据输出（do 是 C++ 关键字，故加下划线） */
    uint16_t bclk;   /*!< 位时钟 */
    uint16_t mclk;   /*!< 主时钟 */
} axk_bsp_i2s_cfg_t;

int axk_bsp_i2s_init(const axk_bsp_i2s_cfg_t *cfg);
int axk_bsp_i2s_deinit(uint8_t port);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_I2S_H */
