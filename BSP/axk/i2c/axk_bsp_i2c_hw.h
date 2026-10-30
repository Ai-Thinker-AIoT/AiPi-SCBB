/**
 * @file axk_bsp_i2c_hw.h
 * @brief axk BSP —— 硬件 I2C 主机（open SDK aiio_i2c 适配）
 *
 * 对应中间件 API：aiio_i2c_master_init/deinit/write/read/write_byte/read_byte
 *
 * 与 axk_bsp_i2c.h 的软件 I2C 并存：
 *   · 软件 I2C 走 GPIO 模拟，任意引脚、任意芯片，适合传感器模块
 *   · 硬件 I2C 只在芯片有对应外设时可用，速率高但引脚受限
 *
 * 约定与差异：
 *   · dev_addr 为 7 位地址（如 0x44），内部左移 1 位后交给中间件
 *   · 配置里的 freq_hz 是总线频率（Hz）；中间件字段名虽叫 period
 *   · BL616CL 只实现了 AIIO_I2C_0，且每次传输后固定延时 100ms
 *   · BL616CL 的 aiio_i2c_master_write_byte() 有指针 bug，
 *     本包装层用 write() + 单字节缓冲实现，不复用它
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_I2C_HW_H
#define AXK_BSP_I2C_HW_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 硬件 I2C 端口（与 aiio_i2c_enum_t 数值一致） */
typedef enum {
    AXK_BSP_I2C_HW_PORT0 = 0,
    AXK_BSP_I2C_HW_PORT1 = 1,
    AXK_BSP_I2C_HW_PORT2 = 2,
} axk_bsp_i2c_hw_port_t;

typedef struct {
    uint8_t  port;      /*!< axk_bsp_i2c_hw_port_t */
    uint16_t sda;       /*!< axk 编码引脚，AXK_BSP_GPIO_PIN(port, pin) */
    uint16_t scl;       /*!< axk 编码引脚 */
    uint32_t freq_hz;   /*!< 总线频率，如 100000 / 400000 */
} axk_bsp_i2c_hw_cfg_t;

/** @brief 初始化硬件 I2C 主机 */
int axk_bsp_i2c_hw_init(const axk_bsp_i2c_hw_cfg_t *cfg);

/** @brief 反初始化硬件 I2C 主机 */
int axk_bsp_i2c_hw_deinit(uint8_t port);

/** @brief 写寄存器 + 数据（reg 可为 NULL/0 表示直接写数据） */
int axk_bsp_i2c_hw_write(uint8_t port, uint8_t dev_addr, const uint8_t *reg, uint8_t reg_len,
                         const uint8_t *buf, uint16_t len, uint32_t timeout);

/** @brief 读寄存器 + 数据（reg 可为 NULL/0 表示直接读数据） */
int axk_bsp_i2c_hw_read(uint8_t port, uint8_t dev_addr, const uint8_t *reg, uint8_t reg_len,
                        uint8_t *buf, uint16_t len, uint32_t timeout);

/** @brief 写一个字节寄存器 */
int axk_bsp_i2c_hw_write_byte(uint8_t port, uint8_t dev_addr, const uint8_t *reg, uint8_t reg_len,
                              uint8_t data, uint32_t timeout);

/** @brief 读一个字节寄存器 */
int axk_bsp_i2c_hw_read_byte(uint8_t port, uint8_t dev_addr, const uint8_t *reg, uint8_t reg_len,
                             uint8_t *data, uint32_t timeout);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_I2C_HW_H */
