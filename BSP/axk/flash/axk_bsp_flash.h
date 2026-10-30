/**
 * @file axk_bsp_flash.h
 * @brief axk BSP —— 片内/片外 Flash（open SDK aiio_flash 适配）
 *
 * 对应中间件 API：aiio_flash_init/deinit/write/write_erase/read/erase/
 *                 open/info_get/close 以及各 *_id_read / status*_read
 *
 * 芯片差异很大：中间件头文件声明了完整接口，但多数芯片只实现了
 * init/deinit/write/read/erase，其余函数运行时返回
 * AXK_BSP_ERR_UNSUPPORTED（弱符号）。
 *
 * 注意：写入前必须按扇区擦除；地址是芯片 Flash 映射地址，不是分区偏移。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_FLASH_H
#define AXK_BSP_FLASH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Flash 逻辑分区信息（与 aiio logic_partition_t 布局一致） */
typedef struct {
    const char *partition_description;
    uint32_t    partition_start_addr;
    uint32_t    partition_length;
    uint32_t    partition_options;
} axk_bsp_flash_partition_t;

int axk_bsp_flash_init(void);
int axk_bsp_flash_deinit(void);
int axk_bsp_flash_write(uint32_t addr, const uint8_t *data, uint32_t len);
int axk_bsp_flash_write_erase(uint32_t addr, const uint8_t *data, uint32_t len);
int axk_bsp_flash_read(uint32_t addr, uint8_t *data, uint32_t len);
int axk_bsp_flash_erase(uint32_t addr, uint32_t len);

/**
 * @brief 打开 Flash 分区
 *
 * @return 中间件原值：0 表示失败；非 0 是分区句柄（BL602/LN882H 上是
 *         设备指针，其余芯片未实现返回 AXK_BSP_ERR_UNSUPPORTED）
 */
int32_t axk_bsp_flash_open(const char *name, unsigned int flags);

/** @brief 读取分区信息（handle 为 open 的返回值） */
int axk_bsp_flash_info_get(int32_t handle, axk_bsp_flash_partition_t *part);

/** @brief 关闭分区（handle 为 open 的返回值） */
int axk_bsp_flash_close(int32_t handle);

int axk_bsp_flash_id_read(void);
int axk_bsp_flash_device_id_read(void);
int axk_bsp_flash_unique_id_read(void);
int axk_bsp_flash_manufacturer_id_read(void);
int axk_bsp_flash_status_read(void);
int axk_bsp_flash_status_register1_read(void);
int axk_bsp_flash_status_register2_read(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_FLASH_H */
