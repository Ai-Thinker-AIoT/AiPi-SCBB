/**
 * @file axk_bsp_pwm.h
 * @brief axk BSP —— PWM 输出（open SDK aiio_pwm 适配）
 *
 * 对应中间件 API：aiio_pwm_init/start/set_duty/get_duty/set_freq/get_freq/
 *                 output_close/output
 *
 * 约定与差异：
 *   · 频率单位统一用 Hz。中间件 aiio_pwm_set_freq() 的头文件注释写的是
 *     “period, 单位 us”，但实现里按 Hz 参与分频计算，这里以实际行为为准
 *   · duty 为百分比 0~100，且 BL 实现不接受 0
 *   · 通道号由 aiio_pwm_init() 按引脚分配并返回，后续操作都用这个通道号
 *   · aiio_pwm_output() 在 BL616CL 上是空实现，调用会返回 AXK_BSP_ERR
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_PWM_H
#define AXK_BSP_PWM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 PWM 输出
 *
 * @param[in]  freq_hz  频率（Hz）
 * @param[in]  duty_pct 占空比 0~100（BL 实现要求 >0）
 * @param[in]  pin      axk 编码引脚
 * @return >=0 通道号；<0 失败
 */
int axk_bsp_pwm_init(uint32_t freq_hz, float duty_pct, uint16_t pin);

/** @brief 启动指定通道输出 */
int axk_bsp_pwm_start(int channel);

/** @brief 设置占空比（%） */
int axk_bsp_pwm_set_duty(int channel, float duty_pct);

/** @brief 读取占空比（%） */
int axk_bsp_pwm_get_duty(int channel, float *duty_pct);

/** @brief 设置频率（Hz） */
int axk_bsp_pwm_set_freq(int channel, uint32_t freq_hz);

/** @brief 读取频率（Hz） */
int axk_bsp_pwm_get_freq(int channel, uint32_t *freq_hz);

/** @brief 关闭指定通道输出并释放引脚 */
int axk_bsp_pwm_output_close(int channel);

/**
 * @brief 单次输出若干周期后停止（中间件语义，BL 未实现）
 *
 * @param[in]  channel  通道号
 * @param[in]  cnt      周期数
 * @param[in]  freq_hz  频率（Hz）
 * @param[in]  duty_pct 占空比（%）
 * @param[in]  pin      axk 编码引脚
 */
int axk_bsp_pwm_output(int channel, int cnt, uint32_t freq_hz, float duty_pct, uint16_t pin);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_PWM_H */
