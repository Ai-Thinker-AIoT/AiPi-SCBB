/**
 * @file axk_bsp_radar.c
 * @brief axk BSP —— RD-01 雷达协议实现（弱引用 aiio_radar API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_radar.h"

#include <stddef.h>

#include "axk_bsp_config.h"

extern int32_t aiio_rd01_init(void) AXK_BSP_WEAK;
extern int32_t aiio_rd01_data_output_cb_register(void (*cb)(uint8_t *, uint16_t)) AXK_BSP_WEAK;
extern int32_t aiio_rd01_cmd_ack_cb_register(void (*cb)(uint8_t *, uint16_t)) AXK_BSP_WEAK;
extern void aiio_radar_cmd_process(uint8_t *buf, uint16_t len) AXK_BSP_WEAK;

int axk_bsp_radar_init(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_rd01_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_rd01_init() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_radar_data_cb_register(axk_bsp_radar_data_cb_t cb)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_rd01_data_output_cb_register)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_rd01_data_output_cb_register(cb) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_radar_ack_cb_register(axk_bsp_radar_ack_cb_t cb)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_rd01_cmd_ack_cb_register)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_rd01_cmd_ack_cb_register(cb) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

void axk_bsp_radar_cmd_process(uint8_t *buf, uint16_t len)
{
    if (buf == NULL || !AXK_BSP_WEAK_AVAILABLE(aiio_radar_cmd_process)) {
        return;
    }
    aiio_radar_cmd_process(buf, len);
}
