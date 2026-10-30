/**
 * @file axk_bsp_rtc.h
 * @brief axk BSP —— RTC 实时时钟（open SDK aiio_rtc 适配）
 *
 * 对应中间件 API：aiio_hal_rtc_init/deinit/get_time/set_time
 *
 * 芯片支持：BS21 / LN882H 没有实现，调用返回 AXK_BSP_ERR_UNSUPPORTED。
 * 时间单位为秒（Unix 时间戳）。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_RTC_H
#define AXK_BSP_RTC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int axk_bsp_rtc_init(void);
int axk_bsp_rtc_deinit(void);

/** @brief 读取 RTC 时间（Unix 秒） */
int axk_bsp_rtc_get_time(uint64_t *timestamp_s);

/** @brief 设置 RTC 时间（Unix 秒） */
int axk_bsp_rtc_set_time(uint64_t timestamp_s);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_RTC_H */
