/**
 * @file axk_bsp_gpio.h
 * @brief axk BSP GPIO 接口（基于 open SDK 中间件 aiio_gpio）
 *
 * 与 Ai-M6x BSP 的 bsp_gpio_* 接口一一对应，模块侧通过 ACLL 宏调用，
 * 实际函数名前缀为 axk_bsp_gpio。
 *
 * 引脚参数编码见 ../axk_bsp_config.h：0x00~0xFF = 端口 A，
 * 0x100|n = 端口 B，0x200|n = 端口 C。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_GPIO_H
#define AXK_BSP_GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 浮空输入 */
#define AXK_BSP_GPIO_MODE_IN       0u
/** @brief 推挽输出 */
#define AXK_BSP_GPIO_MODE_OUT      1u
/** @brief 兼容旧 BSP 命名：SPI MOSI，等同输出 */
#define AXK_BSP_GPIO_MODE_SPI_MOSI 2u
/** @brief 兼容旧 BSP 命名：SPI SCLK，等同输出 */
#define AXK_BSP_GPIO_MODE_SPI_SCLK 3u
/** @brief 上拉输入 */
#define AXK_BSP_GPIO_MODE_IN_PU    4u

/**
 * @brief 初始化 GPIO
 * @param pin  axk 逻辑引脚号（见 axk_bsp_config.h）
 * @param mode AXK_BSP_GPIO_MODE_*
 */
void axk_bsp_gpio_init(uint16_t pin, uint32_t mode);

/** @brief 输出高电平 */
void axk_bsp_gpio_set(uint16_t pin);

/** @brief 输出低电平 */
void axk_bsp_gpio_reset(uint16_t pin);

/**
 * @brief 读引脚电平
 * @return 0 低电平，1 高电平；引脚号非法时返回 -1
 */
int axk_bsp_gpio_read(uint16_t pin);

/**
 * @brief 反初始化 GPIO，恢复复位状态
 * @return 0 成功，AXK_BSP_ERR_UNSUPPORTED 表示该芯片未实现
 */
int axk_bsp_gpio_deinit(uint16_t pin);

/* ── 外部中断 ────────────────────────────────────────────────────── */

/** @brief GPIO 中断回调；param 为中间件透传的参数 */
typedef void (*axk_bsp_gpio_irq_cb_t)(int32_t irq, void *param);

/**
 * @brief 注册 GPIO 外部中断回调
 *
 * ⚠ 触发边沿不由本接口指定：BL 系列按当前上下拉推导
 *   （下拉 → 上升沿，上拉 → 下降沿），因此调用前先用
 *   axk_bsp_gpio_init(pin, AXK_BSP_GPIO_MODE_IN_PU / _IN) 设好上下拉。
 *   其它芯片请查各自 port 实现。
 *
 * @return 0 成功，AXK_BSP_ERR_UNSUPPORTED 表示该芯片未实现
 */
int axk_bsp_gpio_int_register(uint16_t pin, axk_bsp_gpio_irq_cb_t cb);

/**
 * @brief 使能/关闭 GPIO 外部中断
 * @param en 非 0 使能，0 关闭
 */
int axk_bsp_gpio_int_enable(uint16_t pin, uint8_t en);

#ifdef __cplusplus
}
#endif

#endif /* AXK_BSP_GPIO_H */
