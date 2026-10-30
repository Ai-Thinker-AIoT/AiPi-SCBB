/**
 * @file axk_bsp_delay.h
 * @brief axk BSP 延时接口（不依赖具体芯片 SDK）
 *
 * axk 的延时一律是忙等（busy-wait），**绝不调用 RTOS 阻塞接口**，
 * 因此可以在 taskENTER_CRITICAL() / 中断里使用（DHT11 驱动就是这样用的）。
 *
 * us 级精度来自“现场标定”：
 *   · Cortex-M3/M4/M7 用 DWT->CYCCNT（默认开启）
 *   · RISC-V 默认用空转循环标定：BL602 的核没有实现 cycle CSR，
 *     执行 rdcycle 会进非法指令陷入（SDK 默认 handle_trap 是死循环），
 *     确认核支持时才能用 -DAXK_BSP_DELAY_USE_CYCLES=1 打开
 *   · 标定期间以 aiio_os_tick_count()/aiio_os_tick2ms() 为时间基准
 *   · tick 冻结（临界区/中断）或标定失败时退回估算值
 *
 * 建议在进入第一个临界区之前调用一次 axk_bsp_delay_init()，
 * 这样标定一定发生在系统 tick 正常走动的时候。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_DELAY_H
#define AXK_BSP_DELAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 提前做周期计数器标定；重复调用无副作用 */
void axk_bsp_delay_init(void);

/** @brief 毫秒级忙等 */
void axk_bsp_delay_ms(uint32_t ms);

/** @brief 微秒级忙等 */
void axk_bsp_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* AXK_BSP_DELAY_H */
