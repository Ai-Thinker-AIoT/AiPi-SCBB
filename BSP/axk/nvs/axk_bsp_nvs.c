/**
 * @file axk_bsp_nvs.c
 * @brief axk BSP —— NVS 实现（弱引用 aiio_nvs API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_nvs.h"

#include "axk_bsp_config.h"
#include "aiio_nvs.h"

extern int32_t aiio_nvs_init(void) AXK_BSP_WEAK;
extern int32_t aiio_nvs_deinit(void) AXK_BSP_WEAK;
extern aiio_nvs_err_code_t aiio_nvs_erase_key(const char *key) AXK_BSP_WEAK;
extern aiio_nvs_err_code_t aiio_nvs_erase_all(void) AXK_BSP_WEAK;
extern void aiio_nvs_dump(void) AXK_BSP_WEAK;
extern size_t aiio_nvs_get_blob(const char *key, void *value_buf, size_t buf_len,
                                size_t *saved_value_len) AXK_BSP_WEAK;
extern aiio_nvs_err_code_t aiio_nvs_set_blob(const char *key, const void *value_buf,
                                             size_t buf_len) AXK_BSP_WEAK;

int axk_bsp_nvs_init(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_nvs_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_nvs_init() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_nvs_deinit(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_nvs_deinit)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_nvs_deinit() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_nvs_erase_key(const char *key)
{
    if (key == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_nvs_erase_key)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (int)aiio_nvs_erase_key(key);
}

int axk_bsp_nvs_erase_all(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_nvs_erase_all)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (int)aiio_nvs_erase_all();
}

void axk_bsp_nvs_dump(void)
{
    if (AXK_BSP_WEAK_AVAILABLE(aiio_nvs_dump)) {
        aiio_nvs_dump();
    }
}

size_t axk_bsp_nvs_get_blob(const char *key, void *value_buf, size_t buf_len,
                            size_t *saved_len)
{
    if (key == NULL || value_buf == NULL) {
        return 0;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_nvs_get_blob)) {
        return 0;
    }
    return aiio_nvs_get_blob(key, value_buf, buf_len, saved_len);
}

int axk_bsp_nvs_set_blob(const char *key, const void *value_buf, size_t buf_len)
{
    if (key == NULL || value_buf == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_nvs_set_blob)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (int)aiio_nvs_set_blob(key, value_buf, buf_len);
}
