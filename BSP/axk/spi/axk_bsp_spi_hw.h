/**
 * @file axk_bsp_spi_hw.h
 * @brief axk BSP —— 硬件 SPI 主机（open SDK aiio_spi 适配）
 *
 * 对应中间件 API：aiio_spi_master_init/deinit/write/read/write_byte/read_byte/write_and_read
 *
 * 与 axk_bsp_spi.h 的软件 SPI 并存：软件 SPI 任意引脚、任意芯片；
 * 硬件 SPI 速率高，但引脚/模式受芯片限制。
 *
 * 已知限制（以 BL616CL 为例）：
 *   · 只实现 AIIO_SPI_0，固定 1MHz / 模式 0 / 8bit / MSB first
 *   · 中间件没有提供设置频率和模式的接口，包装层也无从设置
 *   · CS 由硬件 SPI 外设控制，init 时即占用配置里的 cs 引脚
 *   · timeout 参数在 BL 实现里被忽略
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_SPI_HW_H
#define AXK_BSP_SPI_HW_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 硬件 SPI 端口（与 aiio_spi_enum_t 数值一致） */
typedef enum {
    AXK_BSP_SPI_HW_PORT0 = 0,
    AXK_BSP_SPI_HW_PORT1 = 1,
    AXK_BSP_SPI_HW_PORT2 = 2,
} axk_bsp_spi_hw_port_t;

typedef struct {
    uint8_t  port;   /*!< axk_bsp_spi_hw_port_t */
    uint16_t clk;    /*!< axk 编码引脚 */
    uint16_t mosi;   /*!< axk 编码引脚 */
    uint16_t miso;   /*!< axk 编码引脚 */
    uint16_t cs;     /*!< axk 编码引脚 */
} axk_bsp_spi_hw_cfg_t;

int axk_bsp_spi_hw_init(const axk_bsp_spi_hw_cfg_t *cfg);
int axk_bsp_spi_hw_deinit(uint8_t port);

int axk_bsp_spi_hw_write(uint8_t port, const uint8_t *addr, uint32_t addr_len,
                         const uint8_t *data, uint32_t len, uint32_t timeout);
int axk_bsp_spi_hw_read(uint8_t port, const uint8_t *addr, uint32_t addr_len,
                        uint8_t *data, uint32_t len, uint32_t timeout);
int axk_bsp_spi_hw_write_byte(uint8_t port, const uint8_t *addr, uint32_t addr_len,
                              uint8_t data, uint32_t timeout);
int axk_bsp_spi_hw_read_byte(uint8_t port, const uint8_t *addr, uint32_t addr_len,
                             uint8_t *data, uint32_t timeout);
int axk_bsp_spi_hw_write_and_read(uint8_t port, const uint8_t *tx, uint8_t *rx,
                                  uint32_t len, uint32_t timeout);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_SPI_HW_H */
