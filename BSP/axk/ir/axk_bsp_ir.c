/**
 * @file axk_bsp_ir.c
 * @brief axk BSP —— 红外接收实现（弱引用 aiio_ir API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_ir.h"

#include "axk_bsp_config.h"
#include "aiio_ir.h"

extern aiio_err_t aiio_ir_rx_gpio_init(uint32_t port, uint16_t pin) AXK_BSP_WEAK;
extern aiio_err_t aiio_ir_rx_init(aiio_ir_rx_cfg_t *cfg) AXK_BSP_WEAK;
extern aiio_err_t aiio_ir_rx_enable(uint8_t enable) AXK_BSP_WEAK;
extern aiio_err_t aiio_ir_receive(uint64_t *rx_data, uint8_t *bit_cnt) AXK_BSP_WEAK;

int axk_bsp_ir_init(uint16_t pin, const axk_bsp_ir_cfg_t *cfg)
{
    aiio_ir_rx_cfg_t rx_cfg;

    if (cfg == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_ir_rx_gpio_init) ||
        !AXK_BSP_WEAK_AVAILABLE(aiio_ir_rx_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }

    if (aiio_ir_rx_gpio_init(axk_bsp_gpio_port_of(pin),
                             axk_bsp_gpio_num_of(pin)) != 0) {
        return AXK_BSP_ERR;
    }

    rx_cfg.input_inverse   = cfg->input_inverse;
    rx_cfg.deglitch_enable = cfg->deglitch_enable;
    rx_cfg.deglitch_cnt    = cfg->deglitch_cnt;
    rx_cfg.mode            = (aiio_rx_mode_t)cfg->mode;
    rx_cfg.data_threshold  = cfg->data_threshold;
    rx_cfg.end_threshold   = cfg->end_threshold;
    rx_cfg.fifo_threshold  = cfg->fifo_threshold;

    return (aiio_ir_rx_init(&rx_cfg) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_ir_enable(uint8_t enable)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_ir_rx_enable)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_ir_rx_enable(enable) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_ir_receive(uint64_t *rx_data, uint8_t *bit_cnt)
{
    if (rx_data == NULL || bit_cnt == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_ir_receive)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_ir_receive(rx_data, bit_cnt) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}
