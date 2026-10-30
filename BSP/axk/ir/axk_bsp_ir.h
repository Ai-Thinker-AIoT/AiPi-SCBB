/**
 * @file axk_bsp_ir.h
 * @brief axk BSP —— 红外接收（open SDK aiio_ir 适配）
 *
 * 对应中间件 API：aiio_ir_rx_gpio_init/rx_init/rx_enable/receive
 *
 * 芯片支持：目前仅 BL602 / BL616 / BL616CL / BL618 实现红外接收，
 * 其余芯片返回 AXK_BSP_ERR_UNSUPPORTED。
 *
 * 已知限制（BL 实现）：
 *   · 只用到 cfg 的 mode / input_inverse / deglitch_enable，
 *     data_threshold / end_threshold / fifo_threshold / deglitch_cnt 被忽略
 *   · 接收引脚由 GPIO 矩阵固定映射到 IR 外设，具体可用 IO 见芯片手册
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_IR_H
#define AXK_BSP_IR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 红外解码模式（与 aiio_rx_mode_t 数值一致） */
typedef enum {
    AXK_BSP_IR_RX_NEC = 0,
    AXK_BSP_IR_RX_RC5,
    AXK_BSP_IR_RX_SWM,
} axk_bsp_ir_rx_mode_t;

typedef struct {
    uint8_t               input_inverse;    /*!< 输入反相 */
    uint8_t               deglitch_enable;  /*!< 去毛刺使能 */
    uint8_t               deglitch_cnt;     /*!< 去毛刺计数（部分芯片忽略） */
    axk_bsp_ir_rx_mode_t  mode;             /*!< NEC / RC5 / 软件脉宽 */
    uint16_t              data_threshold;   /*!< 0/1 脉宽阈值（部分芯片忽略） */
    uint16_t              end_threshold;    /*!< 结束条件阈值（部分芯片忽略） */
    uint16_t              fifo_threshold;   /*!< FIFO 阈值（部分芯片忽略） */
} axk_bsp_ir_cfg_t;

/**
 * @brief 初始化红外接收引脚与接收器
 *
 * @param[in]  pin  axk 编码引脚
 * @param[in]  cfg  接收配置
 */
int axk_bsp_ir_init(uint16_t pin, const axk_bsp_ir_cfg_t *cfg);

/** @brief 使能/关闭接收 */
int axk_bsp_ir_enable(uint8_t enable);

/**
 * @brief 读取一帧红外数据
 *
 * @param[out] rx_data  数据（低位在前）
 * @param[out] bit_cnt  位数
 */
int axk_bsp_ir_receive(uint64_t *rx_data, uint8_t *bit_cnt);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_IR_H */
