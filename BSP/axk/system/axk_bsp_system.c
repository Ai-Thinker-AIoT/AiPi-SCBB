/**
 * @file axk_bsp_system.c
 * @brief axk BSP —— 系统级接口实现（弱引用 aiio API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_system.h"

#include <stddef.h>

#include "axk_bsp_config.h"
#include "aiio_system.h"
#include "aiio_chip_init.h"

extern void aiio_restart(void) AXK_BSP_WEAK;
extern int aiio_random(void) AXK_BSP_WEAK;
extern char *aiio_get_chipname(void) AXK_BSP_WEAK;
extern int32_t aiio_chip_init(void) AXK_BSP_WEAK;

int axk_bsp_system_restart(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_restart)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    aiio_restart();
    return AXK_BSP_OK;
}

int axk_bsp_system_random(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_random)) {
        return 0;
    }
    return aiio_random();
}

const char *axk_bsp_system_chip_name(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_get_chipname)) {
        return NULL;
    }
    return aiio_get_chipname();
}

int axk_bsp_system_chip_init(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_chip_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_chip_init() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}
