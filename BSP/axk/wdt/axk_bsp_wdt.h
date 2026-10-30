/**
 * @file axk_bsp_wdt.h
 * @brief axk BSP —— 看门狗（open SDK aiio_wdt 适配）
 *
 * 对应中间件 API：aiio_wdt_init/init_timeouts/deinit/reload
 *
 * 芯片支持：LN882H 没有实现，调用返回 AXK_BSP_ERR_UNSUPPORTED。
 *
 * 注意：
 *   · BL 实现的 aiio_wdt_init_timeouts() 只是把毫秒数当作比较值，
 *     实际超时取决于 32K 时钟分频，不同芯片语义可能不同
 *   · 重复 init 会被中间件忽略
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_WDT_H
#define AXK_BSP_WDT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int axk_bsp_wdt_init(void);
int axk_bsp_wdt_init_timeouts(uint32_t timeouts_ms);
int axk_bsp_wdt_deinit(void);
int axk_bsp_wdt_reload(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_WDT_H */
