/**
 * @file axk_bsp_pwmcap.c
 * @brief axk BSP —— PWM 捕获实现（弱引用 aiio_pwmcap API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_pwmcap.h"

#include "axk_bsp_config.h"
#include "aiio_pwmcap.h"

extern int32_t aiio_pwmcap_capture_init(aiio_pwmcap_pin_cfg_t *cfg) AXK_BSP_WEAK;
extern int32_t aiio_pwmcap_capture_start(void) AXK_BSP_WEAK;
extern int32_t aiio_pwmcap_capture_close(void) AXK_BSP_WEAK;
extern int32_t aiio_pwmcap_capture_value(uint32_t *freqlist, float *dutylist,
                                         uint32_t *size) AXK_BSP_WEAK;

int axk_bsp_pwmcap_init(const axk_bsp_pwmcap_cfg_t *cfg)
{
    aiio_pwmcap_pin_cfg_t pin_cfg;

    if (cfg == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwmcap_capture_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }

    pin_cfg.aiio_gpio_pwmcap_port = axk_bsp_gpio_port_of(cfg->pin);
    pin_cfg.aiio_gpio_pwmcap_pin  = (uint32_t)axk_bsp_gpio_num_of(cfg->pin);

    return (aiio_pwmcap_capture_init(&pin_cfg) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_pwmcap_start(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwmcap_capture_start)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_pwmcap_capture_start() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_pwmcap_close(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwmcap_capture_close)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_pwmcap_capture_close() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_pwmcap_read(uint32_t *freqlist, float *dutylist, uint32_t *size)
{
    if (freqlist == NULL || dutylist == NULL || size == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwmcap_capture_value)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_pwmcap_capture_value(freqlist, dutylist, size) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}
