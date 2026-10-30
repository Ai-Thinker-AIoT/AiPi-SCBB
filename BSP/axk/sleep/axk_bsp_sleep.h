/**
 * @file axk_bsp_sleep.h
 * @brief axk BSP —— 低功耗/睡眠（open SDK aiio_sleep 适配）
 *
 * 对应中间件 API：aiio_set_low_power_mode()
 *
 * 芯片支持：BS21 没有实现，返回 AXK_BSP_ERR_UNSUPPORTED。
 *
 * 重要差异：
 *   · 中间件 wakeUpPin 是“映射后的引脚序号”，不是 axk 引脚编码，
 *     各芯片含义不同（BL616CL 实现里 GPIO 唤醒被关闭，只支持定时唤醒）
 *   · 返回码沿用中间件约定：0 成功；1 失败；2 模式不支持；3 参数错误；
 *     4 引脚不支持唤醒；128 未适配
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_SLEEP_H
#define AXK_BSP_SLEEP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 低功耗模式（与 aiio_lower_power_mode_t 数值一致） */
typedef enum {
    AXK_BSP_SLEEP_LIGHT = 0,        /*!< 浅睡眠（保持连接） */
    AXK_BSP_SLEEP_LIGHT_AUTO = 1,   /*!< 上电自动浅睡眠 */
    AXK_BSP_SLEEP_DEEP = 2,         /*!< 深睡眠（断开连接） */
    AXK_BSP_SLEEP_NONE = 3,         /*!< 正常工作 */
} axk_bsp_sleep_mode_t;

/** @brief 唤醒源（与 aiio_wakeup_source_mode_t 数值一致） */
typedef enum {
    AXK_BSP_WAKEUP_TIMER = 0,
    AXK_BSP_WAKEUP_GPIO = 2,
} axk_bsp_wakeup_source_t;

typedef struct {
    axk_bsp_sleep_mode_t      mode;
    axk_bsp_wakeup_source_t   wake_source;
    uint32_t                  wake_ms;     /*!< 定时唤醒时间 */
    uint8_t                   wake_pin;    /*!< 中间件映射后的唤醒脚序号 */
    uint8_t                   wake_level;  /*!< 0 低电平，其它高电平 */
} axk_bsp_sleep_cfg_t;

/**
 * @brief 进入低功耗模式
 *
 * @return 中间件返回码（0 成功，见文件头说明）；负数表示参数错误/未实现
 */
int axk_bsp_sleep_enter(const axk_bsp_sleep_cfg_t *cfg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_SLEEP_H */
