/**
 * @file axk_bsp_rtc.c
 * @brief axk BSP —— RTC 实现（弱引用 aiio_rtc API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_rtc.h"

#include "axk_bsp_config.h"
#include "aiio_rtc.h"

extern int32_t aiio_hal_rtc_init(void) AXK_BSP_WEAK;
extern int32_t aiio_hal_rtc_deinit(void) AXK_BSP_WEAK;
extern int32_t aiio_hal_rtc_get_time(uint64_t *time_stamp) AXK_BSP_WEAK;
extern int32_t aiio_hal_rtc_set_time(uint64_t *time_stamp) AXK_BSP_WEAK;

int axk_bsp_rtc_init(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_hal_rtc_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_hal_rtc_init() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_rtc_deinit(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_hal_rtc_deinit)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_hal_rtc_deinit() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_rtc_get_time(uint64_t *timestamp_s)
{
    if (timestamp_s == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_hal_rtc_get_time)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_hal_rtc_get_time(timestamp_s) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_rtc_set_time(uint64_t timestamp_s)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_hal_rtc_set_time)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_hal_rtc_set_time(&timestamp_s) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}
