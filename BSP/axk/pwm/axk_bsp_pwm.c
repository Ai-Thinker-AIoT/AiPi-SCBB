/**
 * @file axk_bsp_pwm.c
 * @brief axk BSP —— PWM 实现（弱引用 aiio_pwm API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_pwm.h"

#include "axk_bsp_config.h"
#include "aiio_pwm.h"

extern int aiio_pwm_init(aiio_pwm_init_t *init) AXK_BSP_WEAK;
extern int aiio_pwm_start(aiio_pwm_chann_t pwm_channel_num) AXK_BSP_WEAK;
extern int aiio_pwm_set_duty(aiio_pwm_chann_t pwm_channel_num, float duty) AXK_BSP_WEAK;
extern float aiio_pwm_get_duty(aiio_pwm_chann_t pwm_channel_num) AXK_BSP_WEAK;
extern int aiio_pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t period) AXK_BSP_WEAK;
extern int32_t aiio_pwm_get_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t *period) AXK_BSP_WEAK;
extern int32_t aiio_pwm_output_close(aiio_pwm_chann_t pwm_channel_num) AXK_BSP_WEAK;
extern int32_t aiio_pwm_output(aiio_pwm_chann_t pwm_channel_num, int cnt, uint32_t freq,
                               float duty, uint32_t gpio_port, uint32_t gpio_pin) AXK_BSP_WEAK;

int axk_bsp_pwm_init(uint32_t freq_hz, float duty_pct, uint16_t pin)
{
    aiio_pwm_init_t cfg;
    int channel;

    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwm_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }

    cfg.freq      = freq_hz;
    cfg.duty      = duty_pct;
    cfg.gpio_port = (uint16_t)axk_bsp_gpio_port_of(pin);
    cfg.gpio_pin  = axk_bsp_gpio_num_of(pin);

    channel = aiio_pwm_init(&cfg);
    return (channel >= 0) ? channel : AXK_BSP_ERR;
}

int axk_bsp_pwm_start(int channel)
{
    if (channel < 0 || channel > 11) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwm_start)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_pwm_start((aiio_pwm_chann_t)channel) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_pwm_set_duty(int channel, float duty_pct)
{
    if (channel < 0 || channel > 11) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwm_set_duty)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_pwm_set_duty((aiio_pwm_chann_t)channel, duty_pct) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_pwm_get_duty(int channel, float *duty_pct)
{
    float duty;

    if (duty_pct == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (channel < 0 || channel > 11) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwm_get_duty)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }

    duty = aiio_pwm_get_duty((aiio_pwm_chann_t)channel);
    if (duty < 0.0f) {
        return AXK_BSP_ERR;
    }
    *duty_pct = duty;
    return AXK_BSP_OK;
}

int axk_bsp_pwm_set_freq(int channel, uint32_t freq_hz)
{
    if (channel < 0 || channel > 11) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwm_set_freq)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_pwm_set_freq((aiio_pwm_chann_t)channel, freq_hz) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_pwm_get_freq(int channel, uint32_t *freq_hz)
{
    if (freq_hz == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (channel < 0 || channel > 11) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwm_get_freq)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_pwm_get_freq((aiio_pwm_chann_t)channel, freq_hz) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_pwm_output_close(int channel)
{
    if (channel < 0 || channel > 11) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwm_output_close)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_pwm_output_close((aiio_pwm_chann_t)channel) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_pwm_output(int channel, int cnt, uint32_t freq_hz, float duty_pct, uint16_t pin)
{
    if (channel < 0 || channel > 11) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_pwm_output)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_pwm_output((aiio_pwm_chann_t)channel, cnt, freq_hz, duty_pct,
                            axk_bsp_gpio_port_of(pin), axk_bsp_gpio_num_of(pin)) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}
