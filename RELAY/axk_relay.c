/**
 * @file axk_relay.c
 * @brief 继电器驱动 — 纯 ACLL 跨平台实现。
 * @author xemowo (xemowo@qq.com)
 * @version 0.1.1
 * @date 2026-07-31
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_relay.h"

#ifdef SCBB_RELAY_ENABLED

#define AXK_RELAY_GPIO_INIT(p,m)  AXK_RELAY_GPIO_ACLL(init, p, m)
#define AXK_RELAY_GPIO_SET(p)     AXK_RELAY_GPIO_ACLL(set, p)
#define AXK_RELAY_GPIO_RESET(p)   AXK_RELAY_GPIO_ACLL(reset, p)

static bool s_axk_relay_state;
static uint16_t s_axk_relay_pin = AXK_RELAY_PIN;

/**
 * @brief 用运行时指定的引脚初始化继电器，默认为释放状态
 *
 * @param pin  继电器控制脚（axk 逻辑引脚号）
 * @return int 0: 成功
 */
int axk_relay_init_pin(uint16_t pin) {
    s_axk_relay_pin = pin;
    AXK_RELAY_GPIO_INIT(s_axk_relay_pin, 1);
    axk_relay_off();
    return 0;
}

/**
 * @brief 用编译期 AXK_RELAY_PIN 初始化继电器
 *
 * @return int  0: 成功
 */
int axk_relay_init(void) {
    return axk_relay_init_pin(AXK_RELAY_PIN);
}

/**
 * @brief 吸合继电器（输出高电平）
 */
void axk_relay_on(void) {
    AXK_RELAY_GPIO_SET(s_axk_relay_pin);
    s_axk_relay_state = true;
}

/**
 * @brief 释放继电器（输出低电平）
 */
void axk_relay_off(void) {
    AXK_RELAY_GPIO_RESET(s_axk_relay_pin);
    s_axk_relay_state = false;
}

/**
 * @brief 翻转继电器状态
 */
void axk_relay_toggle(void) {
    if (s_axk_relay_state) {
        axk_relay_off();
    } else {
        axk_relay_on();
    }
}

#endif /* SCBB_RELAY_ENABLED */
