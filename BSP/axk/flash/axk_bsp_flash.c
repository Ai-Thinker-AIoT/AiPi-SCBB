/**
 * @file axk_bsp_flash.c
 * @brief axk BSP —— Flash 实现（弱引用 aiio_flash API）
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_flash.h"

#include "axk_bsp_config.h"
#include "aiio_flash.h"

extern int32_t aiio_flash_init(void) AXK_BSP_WEAK;
extern int32_t aiio_flash_deinit(void) AXK_BSP_WEAK;
extern int32_t aiio_flash_write(uint32_t write_addr, uint8_t *write_src_data,
                                uint32_t write_length) AXK_BSP_WEAK;
extern int32_t aiio_flash_write_erase(uint32_t addr, uint8_t *src_data,
                                      uint32_t length) AXK_BSP_WEAK;
extern int32_t aiio_flash_read(uint32_t read_addr, uint8_t *read_dst_data,
                               uint32_t read_length) AXK_BSP_WEAK;
extern int32_t aiio_flash_erase(uint32_t erase_addr, uint32_t erase_length) AXK_BSP_WEAK;
extern int32_t aiio_flash_open(const char *name, unsigned int flags) AXK_BSP_WEAK;
extern int32_t aiio_flash_info_get(flash_dev_t *p_dev,
                                   logic_partition_t *partition) AXK_BSP_WEAK;
extern int32_t aiio_flash_close(flash_dev_t *p_dev) AXK_BSP_WEAK;
extern int32_t aiio_flash_id_read(void) AXK_BSP_WEAK;
extern int32_t aiio_flash_device_id_read(void) AXK_BSP_WEAK;
extern int32_t aiio_flash_unique_id_read(void) AXK_BSP_WEAK;
extern int32_t aiio_flash_manufacturer_id_read(void) AXK_BSP_WEAK;
extern int32_t aiio_flash_status_read(void) AXK_BSP_WEAK;
extern int32_t aiio_flash_status_register1_read(void) AXK_BSP_WEAK;
extern int32_t aiio_flash_status_register2_read(void) AXK_BSP_WEAK;

int axk_bsp_flash_init(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_flash_init)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_flash_init() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_flash_deinit(void)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_flash_deinit)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_flash_deinit() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_flash_write(uint32_t addr, const uint8_t *data, uint32_t len)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_flash_write)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_flash_write(addr, (uint8_t *)data, len) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_flash_write_erase(uint32_t addr, const uint8_t *data, uint32_t len)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_flash_write_erase)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_flash_write_erase(addr, (uint8_t *)data, len) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_flash_read(uint32_t addr, uint8_t *data, uint32_t len)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_flash_read)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_flash_read(addr, data, len) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int axk_bsp_flash_erase(uint32_t addr, uint32_t len)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_flash_erase)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_flash_erase(addr, len) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

int32_t axk_bsp_flash_open(const char *name, unsigned int flags)
{
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_flash_open)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    /* 原样返回：BL602/LN882H 的 open 返回的是分区设备指针，不是状态码 */
    return aiio_flash_open(name, flags);
}

int axk_bsp_flash_info_get(int32_t handle, axk_bsp_flash_partition_t *part)
{
    if (handle == 0 || part == NULL) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_flash_info_get)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_flash_info_get((flash_dev_t *)(intptr_t)handle,
                                (logic_partition_t *)part) == 0)
               ? AXK_BSP_OK
               : AXK_BSP_ERR;
}

int axk_bsp_flash_close(int32_t handle)
{
    if (handle == 0) {
        return AXK_BSP_ERR_PARAM;
    }
    if (!AXK_BSP_WEAK_AVAILABLE(aiio_flash_close)) {
        return AXK_BSP_ERR_UNSUPPORTED;
    }
    return (aiio_flash_close((flash_dev_t *)(intptr_t)handle) == 0) ? AXK_BSP_OK : AXK_BSP_ERR;
}

#define AXK_FLASH_ID_WRAP(fn, name)                       \
    int name(void)                                        \
    {                                                     \
        if (!AXK_BSP_WEAK_AVAILABLE(fn)) {                \
            return AXK_BSP_ERR_UNSUPPORTED;               \
        }                                                 \
        return (fn() == 0) ? AXK_BSP_OK : AXK_BSP_ERR;    \
    }

AXK_FLASH_ID_WRAP(aiio_flash_id_read, axk_bsp_flash_id_read)
AXK_FLASH_ID_WRAP(aiio_flash_device_id_read, axk_bsp_flash_device_id_read)
AXK_FLASH_ID_WRAP(aiio_flash_unique_id_read, axk_bsp_flash_unique_id_read)
AXK_FLASH_ID_WRAP(aiio_flash_manufacturer_id_read, axk_bsp_flash_manufacturer_id_read)
AXK_FLASH_ID_WRAP(aiio_flash_status_read, axk_bsp_flash_status_read)
AXK_FLASH_ID_WRAP(aiio_flash_status_register1_read, axk_bsp_flash_status_register1_read)
AXK_FLASH_ID_WRAP(aiio_flash_status_register2_read, axk_bsp_flash_status_register2_read)
