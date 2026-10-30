/**
 * @file axk_bsp_system.h
 * @brief axk BSP —— 系统级接口（open SDK aiio_system / aiio_chip_init 适配）
 *
 * 对应中间件 API：
 *   · aiio_restart()      → axk_bsp_system_restart()
 *   · aiio_random()       → axk_bsp_system_random()
 *   · aiio_get_chipname() → axk_bsp_system_chip_name()
 *   · aiio_chip_init()    → axk_bsp_system_chip_init()
 *
 * 芯片支持：BS21 没有实现 aiio_restart，返回 AXK_BSP_ERR_UNSUPPORTED。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#ifndef AXK_BSP_SYSTEM_H
#define AXK_BSP_SYSTEM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 软复位
 * @return 正常不返回（复位）；返回 AXK_BSP_ERR_UNSUPPORTED 表示该芯片未实现
 */
int axk_bsp_system_restart(void);

/**
 * @brief 取一个随机数
 * @return 随机值；芯片未实现时返回 0（此时可用 AXK_BSP_WEAK_AVAILABLE 自行判断）
 */
int axk_bsp_system_random(void);

/**
 * @brief 取芯片名
 * @return 芯片名字符串，如 "bl616cl"；未实现返回 NULL
 */
const char *axk_bsp_system_chip_name(void);

/**
 * @brief 芯片底层初始化（时钟/寄存器等）
 *
 * ⚠ 应用 main() 里已经调用过一次，正常业务代码不要重复调用。
 *   本接口仅为 BSP 与中间件 API 对齐而提供。
 *
 * @return 0 成功，其他为中间件错误码
 */
int axk_bsp_system_chip_init(void);

#ifdef __cplusplus
}
#endif

#endif /* AXK_BSP_SYSTEM_H */
