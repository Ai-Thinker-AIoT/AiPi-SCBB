/**
 * @file axk_bsp_i2c_hw.c
 * @brief axk BSP —— 硬件 I2C 实现（弱引用 aiio_i2c API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_i2c_hw.h"

#include "axk_bsp_config.h"
#include "aiio_i2c.h"

extern int32_t aiio_i2c_master_init(aiio_i2c_enum_t aiio_i2c_port, aiio_i2c_pin_cfg_t *aiio_i2c_cfg) AXK_BSP_WEAK;
extern int32_t aiio_i2c_master_deinit(aiio_i2c_enum_t aiio_i2c_port) AXK_BSP_WEAK;
extern int32_t aiio_i2c_master_write(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr,
                                     uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len,
                                     uint32_t timeout) AXK_BSP_WEAK;
extern int32_t aiio_i2c_master_read(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr,
                                    uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len,
                                    uint32_t timeout) AXK_BSP_WEAK;

int axk_bsp_i2c_hw_init(const axk_bsp_i2c_hw_cfg_t *cfg)
{
    aiio_i2c_pin_cfg_t pin_cfg;

    if (cfg == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_i2c_master_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }

    pin_cfg.aiio_gpio_sda_port = (uint8_t)axk_bsp_gpio_port_of(cfg->sda);
    pin_cfg.aiio_gpio_sda_pin  = (uint32_t)axk_bsp_gpio_num_of(cfg->sda);
    pin_cfg.aiio_gpio_scl_port = (uint8_t)axk_bsp_gpio_port_of(cfg->scl);
    pin_cfg.aiio_gpio_scl_pin  = (uint32_t)axk_bsp_gpio_num_of(cfg->scl);
    pin_cfg.aiio_i2c_period    = cfg->freq_hz;   /* 中间件实际按 Hz 使用 */

    return (aiio_i2c_master_init((aiio_i2c_enum_t)cfg->port, &pin_cfg) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_i2c_hw_deinit(uint8_t port)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_i2c_master_deinit)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_i2c_master_deinit((aiio_i2c_enum_t)port) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_i2c_hw_write(uint8_t port, uint8_t dev_addr, const uint8_t *reg, uint8_t reg_len,
                         const uint8_t *buf, uint16_t len, uint32_t timeout)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_i2c_master_write)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    /* aiio 的 dev_addr 是 8 位形式（内部 >>1），这里统一收 7 位地址 */
    return (aiio_i2c_master_write((aiio_i2c_enum_t)port, (uint8_t)(dev_addr << 1),
                                  (uint8_t *)reg, reg_len, (uint8_t *)buf, len, timeout) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_i2c_hw_read(uint8_t port, uint8_t dev_addr, const uint8_t *reg, uint8_t reg_len,
                        uint8_t *buf, uint16_t len, uint32_t timeout)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_i2c_master_read)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_i2c_master_read((aiio_i2c_enum_t)port, (uint8_t)(dev_addr << 1),
                                 (uint8_t *)reg, reg_len, buf, len, timeout) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_i2c_hw_write_byte(uint8_t port, uint8_t dev_addr, const uint8_t *reg, uint8_t reg_len,
                              uint8_t data, uint32_t timeout)
{
    /* 不走 aiio_i2c_master_write_byte：BL616CL 实现把 data 当指针传，会写坏内存 */
    return axk_bsp_i2c_hw_write(port, dev_addr, reg, reg_len, &data, 1, timeout);
}

int axk_bsp_i2c_hw_read_byte(uint8_t port, uint8_t dev_addr, const uint8_t *reg, uint8_t reg_len,
                             uint8_t *data, uint32_t timeout)
{
    if (data == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    return axk_bsp_i2c_hw_read(port, dev_addr, reg, reg_len, data, 1, timeout);
}
