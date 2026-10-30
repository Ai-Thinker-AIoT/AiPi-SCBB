/**
 * @file axk_bsp_nvs.h
 * @brief axk BSP —— 非易失存储 NVS（open SDK aiio_nvs 适配）
 *
 * 对应中间件 API：aiio_nvs_init/deinit/erase_key/erase_all/dump/get_blob/set_blob
 *
 * 芯片支持：BS21 没有实现，调用返回 AXK_BSP_ERR_UNSUPPORTED。
 *
 * 返回值约定：
 *   · 状态类接口：0 成功；>0 为中间件 aiio_nvs_err_code_t 错误码；
 *     <0 为 axk 错误码（AXK_BSP_ERR_*）
 *   · get_blob 返回实际读到的字节数，0 表示不存在/未实现
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_NVS_H
#define AXK_BSP_NVS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int axk_bsp_nvs_init(void);
int axk_bsp_nvs_deinit(void);

/** @brief 删除一个键 */
int axk_bsp_nvs_erase_key(const char *key);

/** @brief 清空所有键（恢复默认环境） */
int axk_bsp_nvs_erase_all(void);

/** @brief 打印当前 NVS 内容（调试用） */
void axk_bsp_nvs_dump(void);

/**
 * @brief 读取 blob
 *
 * @param[out] value_buf  缓冲区
 * @param[in]  buf_len    缓冲区大小
 * @param[out] saved_len  实际长度（可为 NULL）
 * @return 实际读到的字节数；0 表示不存在/未实现
 */
size_t axk_bsp_nvs_get_blob(const char *key, void *value_buf, size_t buf_len,
                            size_t *saved_len);

/** @brief 写入 blob */
int axk_bsp_nvs_set_blob(const char *key, const void *value_buf, size_t buf_len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AXK_BSP_NVS_H */
