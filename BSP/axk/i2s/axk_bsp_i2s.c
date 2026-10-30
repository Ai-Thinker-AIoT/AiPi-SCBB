/**
 * @file axk_bsp_i2s.c
 * @brief axk BSP —— I2S 实现（弱引用 aiio_hal_i2s API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_i2s.h"

#include "axk_bsp_config.h"
#include "aiio_i2s.h"

extern int aiio_hal_i2s_init(aiio_i2s_enum_t aiio_i2s_port,
                             aiio_i2s_pin_cfg_t *aiio_i2s_cfg) AXK_BSP_WEAK;
extern int aiio_hal_i2s_deinit(aiio_i2s_enum_t aiio_i2s_port) AXK_BSP_WEAK;

int axk_bsp_i2s_init(const axk_bsp_i2s_cfg_t *cfg)
{
    aiio_i2s_pin_cfg_t pin_cfg;

    if (cfg == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_hal_i2s_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }

    pin_cfg.aiio_gpio_fs_port   = (uint8_t)axk_bsp_gpio_port_of(cfg->fs);
    pin_cfg.aiio_gpio_fs_pin    = (uint32_t)axk_bsp_gpio_num_of(cfg->fs);
    pin_cfg.aiio_gpio_di_port   = (uint8_t)axk_bsp_gpio_port_of(cfg->di);
    pin_cfg.aiio_gpio_di_pin    = (uint32_t)axk_bsp_gpio_num_of(cfg->di);
    pin_cfg.aiio_gpio_do_port   = (uint8_t)axk_bsp_gpio_port_of(cfg->do_);
    pin_cfg.aiio_gpio_do_pin    = (uint32_t)axk_bsp_gpio_num_of(cfg->do_);
    pin_cfg.aiio_gpio_bclk_port = (uint8_t)axk_bsp_gpio_port_of(cfg->bclk);
    pin_cfg.aiio_gpio_bclk_pin  = (uint32_t)axk_bsp_gpio_num_of(cfg->bclk);
    pin_cfg.aiio_gpio_mclk_port = (uint8_t)axk_bsp_gpio_port_of(cfg->mclk);
    pin_cfg.aiio_gpio_mclk_pin  = (uint32_t)axk_bsp_gpio_num_of(cfg->mclk);

    return (aiio_hal_i2s_init((aiio_i2s_enum_t)cfg->port, &pin_cfg) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_i2s_deinit(uint8_t port)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_hal_i2s_deinit)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_hal_i2s_deinit((aiio_i2s_enum_t)port) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}
