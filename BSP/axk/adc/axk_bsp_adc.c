/**
 * @file axk_bsp_adc.c
 * @brief axk BSP —— ADC 实现（弱引用 aiio_adc API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_adc.h"

#include "axk_bsp_config.h"
#include "aiio_adc.h"

extern int32_t aiio_adc_init(aiio_adc_cfg_t *aiio_adc_cfg) AXK_BSP_WEAK;
extern int32_t aiio_adc_get_value(aiio_adc_change_t change) AXK_BSP_WEAK;

int axk_bsp_adc_init(const axk_bsp_adc_cfg_t *cfg)
{
    aiio_adc_cfg_t aiio_cfg;

    if (cfg == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_adc_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }

    /* axk 枚举与 aiio 枚举数值一一对应，直接转换 */
    aiio_cfg.change     = (aiio_adc_change_t)cfg->channel;
    aiio_cfg.mode       = (aiio_adc_mode_t)cfg->mode;
    aiio_cfg.resolution = (aiio_adc_resolution_t)cfg->resolution;
    aiio_cfg.vref       = (aiio_adc_vref_t)cfg->vref;

    return (aiio_adc_init(&aiio_cfg) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_adc_read(axk_bsp_adc_channel_t channel)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_adc_get_value)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (int)aiio_adc_get_value((aiio_adc_change_t)channel);
}
