/**
 * @file axk_bsp_sleep.c
 * @brief axk BSP —— 低功耗实现（弱引用 aiio_sleep API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_sleep.h"

#include "axk_bsp_config.h"
#include "aiio_sleep.h"

extern uint8_t aiio_set_low_power_mode(aiio_wakeup_cfg_t *wakeup_cfg) AXK_BSP_WEAK;

int axk_bsp_sleep_enter(const axk_bsp_sleep_cfg_t *cfg)
{
    aiio_wakeup_cfg_t aiio_cfg;

    if (cfg == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_set_low_power_mode)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }

    aiio_cfg.lowerPowerMode   = (aiio_lower_power_mode_t)cfg->mode;
    aiio_cfg.wakeUpSourceMode = (aiio_wakeup_source_mode_t)cfg->wake_source;
    aiio_cfg.wakeUpMs         = cfg->wake_ms;
    aiio_cfg.wakeUpPin        = cfg->wake_pin;
    aiio_cfg.gpioWakeUpLeve   = cfg->wake_level;

    return (int)aiio_set_low_power_mode(&aiio_cfg);
}
