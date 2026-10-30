/**
 * @file axk_bsp_pwmcap.h
 * @brief axk BSP —— PWM 捕获（open SDK aiio_pwmcap 适配）
 *
 * 对应中间件 API：aiio_pwmcap_capture_init/start/close/value
 *
 * 芯片支持：目前仅 BL602 / LN882H 实现，其余芯片调用返回
 * AXK_BSP_ERR_UNSUPPORTED。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_PWMCAP_H
#define AXK_BSP_PWMCAP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t pin;   /*!< axk 编码引脚 */
} axk_bsp_pwmcap_cfg_t;

/** @brief 初始化 PWM 捕获引脚 */
int axk_bsp_pwmcap_init(const axk_bsp_pwmcap_cfg_t *cfg);

/** @brief 开始捕获 */
int axk_bsp_pwmcap_start(void);

/** @brief 停止捕获 */
int axk_bsp_pwmcap_close(void);

/**
 * @brief 读取捕获结果
 *
 * @param[out] freqlist  频率数组
 * @param[out] dutylist  占空比数组（%）
 * @param[in,out] size   输入数组容量，输出实际数量
 */
int axk_bsp_pwmcap_read(uint32_t *freqlist, float *dutylist, uint32_t *size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_PWMCAP_H */
