/**
 * @file axk_bsp_radar.h
 * @brief axk BSP —— RD-01 雷达协议接口（open SDK aiio_radar 适配）
 *
 * 对应中间件 API：
 *   · aiio_rd01_init()                  → axk_bsp_radar_init()
 *   · aiio_rd01_data_output_cb_register() → axk_bsp_radar_data_cb_register()
 *   · aiio_rd01_cmd_ack_cb_register()   → axk_bsp_radar_ack_cb_register()
 *   · aiio_radar_cmd_process()          → axk_bsp_radar_cmd_process()
 *
 * 芯片支持：只有 bl602 的 port 实现了该协议，且依赖
 * `CONFIG_AIIO_RADAR=y`；其它芯片返回 AXK_BSP_ERR_UNSUPPORTED。
 *
 * 注意：本文件**不包含** `aiio_radar.h`——那个头依赖应用构建生成的
 * `aiio_autoconf.h`，并且只在 `CONFIG_AIIO_RADAR` 打开时声明 API。
 * axk 这里用弱引用 + 自定义回调类型，保证任何芯片/配置下都能编译。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_RADAR_H
#define AXK_BSP_RADAR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 雷达数据输出回调 */
typedef void (*axk_bsp_radar_data_cb_t)(uint8_t *data, uint16_t len);

/** @brief 雷达命令应答回调 */
typedef void (*axk_bsp_radar_ack_cb_t)(uint8_t *data, uint16_t len);

/**
 * @brief 初始化 RD-01 雷达
 * @return 0 成功；AXK_BSP_ERR_UNSUPPORTED 当前芯片/配置不支持
 */
int axk_bsp_radar_init(void);

/** @brief 注册雷达数据输出回调 */
int axk_bsp_radar_data_cb_register(axk_bsp_radar_data_cb_t cb);

/** @brief 注册雷达命令应答回调 */
int axk_bsp_radar_ack_cb_register(axk_bsp_radar_ack_cb_t cb);

/** @brief 处理一帧雷达命令（不支持时静默丢弃） */
void axk_bsp_radar_cmd_process(uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* AXK_BSP_RADAR_H */
