/**
 * @file axk_bsp_spi_hw.c
 * @brief axk BSP —— 硬件 SPI 实现（弱引用 aiio_spi API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_spi_hw.h"

#include "axk_bsp_config.h"
#include "aiio_spi.h"

extern int32_t aiio_spi_master_init(aiio_spi_enum_t aiio_spi_port, aiio_spi_pin_cfg_t *aiio_spi_cfg) AXK_BSP_WEAK;
extern int32_t aiio_spi_master_deinit(aiio_spi_enum_t aiio_spi_port) AXK_BSP_WEAK;
extern int32_t aiio_spi_master_write(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len,
                                     uint8_t *send_data, uint32_t data_len, uint32_t timeout) AXK_BSP_WEAK;
extern int32_t aiio_spi_master_read(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len,
                                    uint8_t *rec_data, uint32_t data_len, uint32_t timeout) AXK_BSP_WEAK;
extern int32_t aiio_spi_master_write_and_read(aiio_spi_enum_t aiio_spi_port, uint8_t *send_data,
                                              uint8_t *rec_data, uint32_t data_len,
                                              uint32_t timeout) AXK_BSP_WEAK;

int axk_bsp_spi_hw_init(const axk_bsp_spi_hw_cfg_t *cfg)
{
    aiio_spi_pin_cfg_t pin_cfg;

    if (cfg == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_spi_master_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }

    pin_cfg.aiio_gpio_clk_port  = axk_bsp_gpio_port_of(cfg->clk);
    pin_cfg.aiio_gpio_clk_pin   = (uint32_t)axk_bsp_gpio_num_of(cfg->clk);
    pin_cfg.aiio_gpio_mosi_port = axk_bsp_gpio_port_of(cfg->mosi);
    pin_cfg.aiio_gpio_mosi_pin  = (uint32_t)axk_bsp_gpio_num_of(cfg->mosi);
    pin_cfg.aiio_gpio_miso_port = axk_bsp_gpio_port_of(cfg->miso);
    pin_cfg.aiio_gpio_miso_pin  = (uint32_t)axk_bsp_gpio_num_of(cfg->miso);
    pin_cfg.aiio_gpio_cs_port   = axk_bsp_gpio_port_of(cfg->cs);
    pin_cfg.aiio_gpio_cs_pin    = (uint32_t)axk_bsp_gpio_num_of(cfg->cs);

    return (aiio_spi_master_init((aiio_spi_enum_t)cfg->port, &pin_cfg) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_spi_hw_deinit(uint8_t port)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_spi_master_deinit)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_spi_master_deinit((aiio_spi_enum_t)port) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_spi_hw_write(uint8_t port, const uint8_t *addr, uint32_t addr_len,
                         const uint8_t *data, uint32_t len, uint32_t timeout)
{
    /* timeout 在 BL 实现里被忽略，保留参数是为了接口对齐 */
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_spi_master_write)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_spi_master_write((aiio_spi_enum_t)port, (uint8_t *)addr, addr_len,
                                  (uint8_t *)data, len, timeout) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_spi_hw_read(uint8_t port, const uint8_t *addr, uint32_t addr_len,
                        uint8_t *data, uint32_t len, uint32_t timeout)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_spi_master_read)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_spi_master_read((aiio_spi_enum_t)port, (uint8_t *)addr, addr_len,
                                 data, len, timeout) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_spi_hw_write_byte(uint8_t port, const uint8_t *addr, uint32_t addr_len,
                              uint8_t data, uint32_t timeout)
{
    return axk_bsp_spi_hw_write(port, addr, addr_len, &data, 1, timeout);
}

int axk_bsp_spi_hw_read_byte(uint8_t port, const uint8_t *addr, uint32_t addr_len,
                             uint8_t *data, uint32_t timeout)
{
    if (data == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    return axk_bsp_spi_hw_read(port, addr, addr_len, data, 1, timeout);
}

int axk_bsp_spi_hw_write_and_read(uint8_t port, const uint8_t *tx, uint8_t *rx,
                                  uint32_t len, uint32_t timeout)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_spi_master_write_and_read)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_spi_master_write_and_read((aiio_spi_enum_t)port, (uint8_t *)tx, rx,
                                           len, timeout) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}
