/**
 * @file axk_bsp_i2c.h
 * @brief axk BSP I2C 接口（软件模拟，无芯片依赖）
 *
 * 提供两套接口，对应 SCBB 模块的两种调用方式：
 *
 * 1. 事务式（INA226 用）
 *      int axk_bsp_i2c_init (const char *name, bsp_i2c_cfg_t *cfg);
 *      int axk_bsp_i2c_write(const char *name, uint8_t addr, const uint8_t *data, uint32_t len);
 *      int axk_bsp_i2c_read (const char *name, uint8_t addr, uint8_t *data, uint32_t len);
 *
 * 2. 软时序（CH224A / SHT3x 用）
 *      axk_bsp_i2c_soft_init/start/stop/wait_ack/send_byte/read_byte/send_ack
 *
 * 两套都用 GPIO 模拟（中间件 aiio 的硬件 I2C 在各芯片上行为不一致：
 * bl616cl 无法做“写寄存器后直接读”，RTL 系列引脚固定，软件模拟最稳）。
 *
 * 电气约定：SCL 恒为推挽输出；SDA 用“切方向”模拟开漏——
 *   释放 = 输入 + 上拉，输出 0 = 推挽输出低。SDA 必须外部上拉
 *   （传感器模块板一般自带；裸芯片需 4.7k~10k 到 3.3V）。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_I2C_H
#define AXK_BSP_I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BSP_I2C_CFG_T_DEFINED
#define BSP_I2C_CFG_T_DEFINED
/** @brief I2C 配置结构体（与 Ai-M6x BSP 保持一致） */
typedef struct {
    uint32_t freq;  /**< I2C 时钟频率（Hz） */
} bsp_i2c_cfg_t;
#endif

/* ── 事务式接口 ───────────────────────────────────────────────────── */

/**
 * @brief 初始化事务式 I2C 总线（引脚取 AXK_BSP_I2C_SCL_PIN/SDA_PIN）
 * @param name 设备名，axk 只维护一条总线，名字仅作标识
 * @param cfg  配置，freq 为 0 时用 AXK_BSP_I2C_FREQ
 * @return 0 成功，-1 参数错误
 */
int axk_bsp_i2c_init(const char *name, bsp_i2c_cfg_t *cfg);

/**
 * @brief 初始化事务式 I2C 总线并显式指定引脚（运行时可改，优先于板级表）
 *
 * @param name 设备名，axk 只维护一条总线，名字仅作标识
 * @param scl  SCL 引脚（axk 逻辑引脚号）
 * @param sda  SDA 引脚（axk 逻辑引脚号）
 * @param cfg  配置，freq 为 0 时用 AXK_BSP_I2C_FREQ
 * @return 0 成功，-1 参数错误
 */
int axk_bsp_i2c_init_ex(const char *name, uint16_t scl, uint16_t sda,
                        bsp_i2c_cfg_t *cfg);

/** @brief 写从设备（内部含 START/地址/数据/STOP） */
int axk_bsp_i2c_write(const char *name, uint8_t addr, const uint8_t *data, uint32_t len);

/** @brief 读从设备（内部含 START/地址/读/STOP，最后一字节回 NACK） */
int axk_bsp_i2c_read(const char *name, uint8_t addr, uint8_t *data, uint32_t len);

/* ── 软时序接口 ───────────────────────────────────────────────────── */

/** @brief 初始化软 I2C 引脚（取 AXK_BSP_I2C_SOFT_*，重复调用无副作用） */
void axk_bsp_i2c_soft_init(void);

/**
 * @brief 初始化软 I2C 引脚并显式指定（运行时可改，优先于板级表）
 *
 * @param scl      SCL 引脚（axk 逻辑引脚号）
 * @param sda      SDA 引脚（axk 逻辑引脚号）
 * @param delay_us 半周期延时（us），5 约 100kHz
 * @note  重复调用会按新引脚重新初始化
 */
void axk_bsp_i2c_soft_init_ex(uint16_t scl, uint16_t sda, uint32_t delay_us);

/** @brief 发送起始信号 */
void axk_bsp_i2c_soft_start(void);

/** @brief 发送停止信号 */
void axk_bsp_i2c_soft_stop(void);

/** @brief 等待从机应答 @return 0 = ACK，1 = NACK */
int axk_bsp_i2c_soft_wait_ack(void);

/** @brief 发送 1 字节（MSB 先行） @return 0 */
int axk_bsp_i2c_soft_send_byte(uint8_t data);

/** @brief 读 1 字节（MSB 先行） */
uint8_t axk_bsp_i2c_soft_read_byte(void);

/** @brief 发送应答位 @param ack 0 = ACK（拉低），非 0 = NACK（释放） */
void axk_bsp_i2c_soft_send_ack(int ack);

#ifdef __cplusplus
}
#endif

#endif /* AXK_BSP_I2C_H */
