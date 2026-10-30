/**
 * @file axk_bsp_adc.h
 * @brief axk BSP —— ADC（open SDK aiio_adc 适配）
 *
 * 对应中间件 API：aiio_adc_init() / aiio_adc_get_value()
 *
 * 芯片差异：
 *   · 通道 → 引脚映射由中间件按芯片固定（BL616CL: CH0=IO20 … CH10=IO27）
 *   · BL 系 get_value() 返回毫伏；其他芯片可能返回原始码值
 *   · BL616CL 一次只能配置并读取一个通道，切换通道需重新 init
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_ADC_H
#define AXK_BSP_ADC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief ADC 通道（与 aiio_adc_change_t 数值一致） */
typedef enum {
    AXK_BSP_ADC_CH0 = 0,
    AXK_BSP_ADC_CH1,
    AXK_BSP_ADC_CH2,
    AXK_BSP_ADC_CH3,
    AXK_BSP_ADC_CH4,
    AXK_BSP_ADC_CH5,
    AXK_BSP_ADC_CH6,
    AXK_BSP_ADC_CH7,
    AXK_BSP_ADC_CH8,
    AXK_BSP_ADC_CH9,
    AXK_BSP_ADC_CH10,
    AXK_BSP_ADC_CH11,
} axk_bsp_adc_channel_t;

/** @brief 转换模式（与 aiio_adc_mode_t 数值一致） */
typedef enum {
    AXK_BSP_ADC_MODE_ONE_SHOT = 0,
    AXK_BSP_ADC_MODE_CONTINUOUS = 1,
} axk_bsp_adc_mode_t;

/** @brief 分辨率（与 aiio_adc_resolution_t 数值一致） */
typedef enum {
    AXK_BSP_ADC_RES_8 = 0,
    AXK_BSP_ADC_RES_9,
    AXK_BSP_ADC_RES_10,
    AXK_BSP_ADC_RES_11,
    AXK_BSP_ADC_RES_12,
    AXK_BSP_ADC_RES_14,
    AXK_BSP_ADC_RES_16,
} axk_bsp_adc_resolution_t;

/** @brief 参考电压（与 aiio_adc_vref_t 数值一致） */
typedef enum {
    AXK_BSP_ADC_VREF_0_8V = 0,    /*!< LN882H */
    AXK_BSP_ADC_VREF_0_85V,       /*!< LN882H */
    AXK_BSP_ADC_VREF_0_95V,       /*!< LN882H */
    AXK_BSP_ADC_VREF_1_05V,       /*!< LN882H */
    AXK_BSP_ADC_VREF_2_0V,        /*!< BL616/BL618 */
    AXK_BSP_ADC_VREF_3_2V,        /*!< BL616/BL618 */
    AXK_BSP_ADC_VREF_VCC = 0xFF,
} axk_bsp_adc_vref_t;

typedef struct {
    axk_bsp_adc_channel_t    channel;
    axk_bsp_adc_mode_t       mode;
    axk_bsp_adc_resolution_t resolution;
    axk_bsp_adc_vref_t       vref;
} axk_bsp_adc_cfg_t;

/**
 * @brief 初始化 ADC 通道
 *
 * @param[in]  cfg  通道/模式/分辨率/参考电压
 * @return 0 成功；AXK_BSP_ERR_PARAM 参数为空；AXK_BSP_ERR_UNSUPPORTED 芯片未实现
 */
int axk_bsp_adc_init(const axk_bsp_adc_cfg_t *cfg);

/**
 * @brief 读取一次 ADC 值
 *
 * @param[in]  channel  通道（部分芯片忽略此参数，读最近一次 init 的通道）
 * @return >=0 转换结果（BL 系为毫伏）；<0 失败或芯片未实现
 */
int axk_bsp_adc_read(axk_bsp_adc_channel_t channel);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_ADC_H */
