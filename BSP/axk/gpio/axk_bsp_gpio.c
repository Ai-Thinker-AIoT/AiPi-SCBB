/**
 * @file axk_bsp_gpio.c
 * @brief axk BSP GPIO 实现（基于 open SDK 中间件 aiio_gpio）
 *
 * 各芯片 port 实现差异较大，这里的调用顺序对全部已适配芯片都成立：
 *   1. aiio_hal_gpio_init()        —— BS21 等要求先 init 才能改方向；
 *                                     bl616cl 重复 init 会返回错误，忽略即可
 *   2. aiio_hal_gpio_pin_direction_set()
 *   3. aiio_hal_gpio_pin_pull_set()
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_gpio.h"

#include "axk_bsp_config.h"
#include "aiio_gpio.h"

extern aiio_ret_t aiio_hal_gpio_deinit(uint32_t port, uint16_t pin) AXK_BSP_WEAK;
extern aiio_ret_t aiio_hal_gpio_int_register(uint32_t port, uint16_t pin,
                                             aiio_hal_gpio_int_cb_t cb) AXK_BSP_WEAK;
extern aiio_ret_t aiio_hal_gpio_int_enable(uint32_t port, uint16_t pin,
                                           uint8_t en) AXK_BSP_WEAK;

void axk_bsp_gpio_init(uint16_t pin, uint32_t mode)
{
    uint32_t port = axk_bsp_gpio_port_of(pin);
    uint16_t num  = axk_bsp_gpio_num_of(pin);

    /*
     * init 的返回值不判断：bl616cl 对已初始化的引脚返回失败，
     * 但后续 direction/pull 仍然有效；RTL 系列未支持的引脚会在
     * direction_set 里直接解引用空表项，这里无法补救，只能靠调用方
     * 选对引脚（见 README 的芯片限制）。
     */
    (void)aiio_hal_gpio_init(port, num);

    if (mode == AXK_BSP_GPIO_MODE_IN_PU) {
        (void)aiio_hal_gpio_pin_direction_set(port, num, AIIO_GPIO_INPUT);
        (void)aiio_hal_gpio_pin_pull_set(port, num, AIIO_GPIO_PULL_UP);
        return;
    }

    if (mode == AXK_BSP_GPIO_MODE_IN) {
        (void)aiio_hal_gpio_pin_direction_set(port, num, AIIO_GPIO_INPUT);
        (void)aiio_hal_gpio_pin_pull_set(port, num, AIIO_GPIO_PULL_NONE);
        return;
    }

    /* OUT / SPI_MOSI / SPI_SCLK 统一按推挽输出处理 */
    (void)aiio_hal_gpio_pin_direction_set(port, num, AIIO_GPIO_OUTPUT);
    (void)aiio_hal_gpio_pin_pull_set(port, num, AIIO_GPIO_PULL_NONE);
}

void axk_bsp_gpio_set(uint16_t pin)
{
    (void)aiio_hal_gpio_set(axk_bsp_gpio_port_of(pin),
                            axk_bsp_gpio_num_of(pin), 1);
}

void axk_bsp_gpio_reset(uint16_t pin)
{
    (void)aiio_hal_gpio_set(axk_bsp_gpio_port_of(pin),
                            axk_bsp_gpio_num_of(pin), 0);
}

int axk_bsp_gpio_read(uint16_t pin)
{
    uint8_t level;

    level = aiio_hal_gpio_get(axk_bsp_gpio_port_of(pin),
                              axk_bsp_gpio_num_of(pin));

    return (level > 1u) ? -1 : (int)level;
}

int axk_bsp_gpio_deinit(uint16_t pin)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_hal_gpio_deinit)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_hal_gpio_deinit(axk_bsp_gpio_port_of(pin),
                                 axk_bsp_gpio_num_of(pin)) == AIIO_SUCC)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_gpio_int_register(uint16_t pin, axk_bsp_gpio_irq_cb_t cb)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_hal_gpio_int_register)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_hal_gpio_int_register(axk_bsp_gpio_port_of(pin),
                                       axk_bsp_gpio_num_of(pin),
                                       (aiio_hal_gpio_int_cb_t)cb) == AIIO_SUCC)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_gpio_int_enable(uint16_t pin, uint8_t en)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_hal_gpio_int_enable)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_hal_gpio_int_enable(axk_bsp_gpio_port_of(pin),
                                     axk_bsp_gpio_num_of(pin), en) == AIIO_SUCC)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}
