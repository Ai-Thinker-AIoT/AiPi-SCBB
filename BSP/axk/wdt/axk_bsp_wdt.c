/**
 * @file axk_bsp_wdt.c
 * @brief axk BSP —— 看门狗实现（弱引用 aiio_wdt API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_wdt.h"

#include "axk_bsp_config.h"
#include "aiio_wdt.h"

extern void aiio_wdt_init(void) AXK_BSP_WEAK;
extern void aiio_wdt_init_timeouts(uint32_t timeouts_ms) AXK_BSP_WEAK;
extern void aiio_wdt_deinit(void) AXK_BSP_WEAK;
extern void aiio_wdt_reload(void) AXK_BSP_WEAK;

int axk_bsp_wdt_init(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_wdt_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    aiio_wdt_init();
    return AXK_BSP_OK;
}

int axk_bsp_wdt_init_timeouts(uint32_t timeouts_ms)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_wdt_init_timeouts)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    aiio_wdt_init_timeouts(timeouts_ms);
    return AXK_BSP_OK;
}

int axk_bsp_wdt_deinit(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_wdt_deinit)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    aiio_wdt_deinit();
    return AXK_BSP_OK;
}

int axk_bsp_wdt_reload(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_wdt_reload)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    aiio_wdt_reload();
    return AXK_BSP_OK;
}
