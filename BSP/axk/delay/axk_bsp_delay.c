/**
 * @file axk_bsp_delay.c
 * @brief axk BSP 忙等延时实现（周期计数器 + 现场标定）
 *
 * 为什么不用 aiio_os_tick_dealy()：
 *   FreeRTOS 的 vTaskDelay 在 taskENTER_CRITICAL() 里不会让出 CPU，
 *   实际会一直自旋到超时，行为不可控；DHT11 这类时序驱动还必须保证
 *   30us 级别的精度。所以这里只用硬件周期计数器。
 *
 * 标定流程（axk_bsp_delay_init 或第一次调用延时函数时执行一次）：
 *   1. 探测周期计数器是否真的在走（读一次 → 空转 1000 次 → 再读一次）
 *   2. 以 aiio_os_tick_count() 为基准，测 ~20ms 内走了多少个周期
 *   3. cycles_per_us = 周期数 / 实测微秒数
 *
 * 退化路径：
 *   · 有周期计数器但标定失败（tick 冻结）→ 用 AXK_BSP_CPU_MHZ 估算
 *   · 无周期计数器 → 用空转循环次数标定；仍失败则用
 *     AXK_BSP_DELAY_FALLBACK_LOOPS_PER_US 估算
 *
 * 注意：AXK_BSP_CPU_MHZ 只是“标定失败时的兜底”，正常路径会自动测出
 * 真实主频，不需要手动填。DHT11 对 30us 精度敏感，强烈建议在临界区
 * 之前调用 axk_bsp_delay_init()。
 *
 * @author Ai-Thinker-Open
 * @version 0.1.0
 * @date 2026-10-29
 *
 * @copyright Copyright (c) 2026
 */
#include "axk_bsp_delay.h"

#include "axk_bsp_config.h"
#include "aiio_os_port.h"

/* ── 可覆盖的配置 ─────────────────────────────────────────────────── */

/** @brief 标定窗口（ms），越大越准，首次调用耗时也越长 */
#ifndef AXK_BSP_DELAY_CAL_MS
#define AXK_BSP_DELAY_CAL_MS            20u
#endif

/** @brief 标定失败且无可用周期计数器时的兜底主频（MHz） */
#ifndef AXK_BSP_CPU_MHZ
#define AXK_BSP_CPU_MHZ                 320u
#endif

/** @brief 既无周期计数器又无法标定时的空转循环次数/微秒（粗估） */
#ifndef AXK_BSP_DELAY_FALLBACK_LOOPS_PER_US
#define AXK_BSP_DELAY_FALLBACK_LOOPS_PER_US 64u
#endif

/** @brief 单片延时上限，避免 32 位周期计数溢出（us） */
#define AXK_BSP_DELAY_CHUNK_US          50000u

/* ── 是否启用硬件周期计数器 ─────────────────────────────────────────
 *
 * BL602（Ai-WB2）的 RISC-V 核没有实现 cycle/mcycle CSR：执行 rdcycle 会
 * 触发非法指令异常，而 SDK 默认的 handle_trap 是死循环 —— 现象就是第一次
 * 标定时程序直接“跑飞”，串口停在启动日志末尾，没有任何报错。
 *
 * 因此 RISC-V 默认关闭周期计数器，改用空转循环标定；Cortex-M3/M4/M7 的
 * DWT->CYCCNT 稳定可用，默认打开。确认某颗 RISC-V 核支持 cycle CSR 时，
 * 可在编译选项里加 -DAXK_BSP_DELAY_USE_CYCLES=1。
 */
#ifndef AXK_BSP_DELAY_USE_CYCLES
#if defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
#define AXK_BSP_DELAY_USE_CYCLES        1
#else
#define AXK_BSP_DELAY_USE_CYCLES        0
#endif
#endif

/* ── 周期计数器 ───────────────────────────────────────────────────── */

static uint8_t  s_delay_state;        /* 0 = 未标定，1 = 已标定 */
static uint8_t  s_use_cycles;         /* 1 = 用硬件周期计数器，0 = 用空转循环 */
static uint32_t s_cycles_per_us;
static uint32_t s_loops_per_us;

static void axk_bsp_nop_loop(uint32_t n)
{
    volatile uint32_t sink = 0;

    while (n-- > 0u) {
        sink++;
    }
    (void)sink;
}

static uint8_t axk_bsp_cycles_supported(void)
{
#if AXK_BSP_DELAY_USE_CYCLES
    /* Cortex-M0/M0+ 没有 DWT->CYCCNT，M3/M4/M7 才有；
     * RISC-V 需显式确认该核实现了 cycle CSR */
    return 1u;
#else
    return 0u;
#endif
}

static void axk_bsp_cycles_enable(void)
{
#if AXK_BSP_DELAY_USE_CYCLES && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    /* DEMCR.TRCENA = 1, DWT_CTRL.CYCCNTENA = 1 */
    *(volatile uint32_t *)0xE000EDFCu |= (1u << 24);
    *(volatile uint32_t *)0xE0001000u |= 1u;
    *(volatile uint32_t *)0xE0001004u = 0u;
#endif
}

static uint32_t axk_bsp_cycles(void)
{
#if !AXK_BSP_DELAY_USE_CYCLES
    return 0u;                 /* 关闭时连指令都不生成，避免 rdcycle 陷入 */
#elif defined(__riscv)
    uint32_t c;

    __asm__ __volatile__("rdcycle %0" : "=r"(c));
    return c;
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
    return *(volatile uint32_t *)0xE0001004u;
#else
    return 0u;
#endif
}

/* ── 标定 ─────────────────────────────────────────────────────────── */

static void axk_bsp_delay_fallback(uint8_t has_cycles)
{
    if (has_cycles) {
        s_cycles_per_us = AXK_BSP_CPU_MHZ;
        s_use_cycles    = 1u;
    } else {
        s_loops_per_us  = AXK_BSP_DELAY_FALLBACK_LOOPS_PER_US;
        s_use_cycles    = 0u;
    }
    s_delay_state = 1u;
}

static void axk_bsp_delay_calibrate(void)
{
    uint32_t t0 = 0u, t1 = 0u;
    uint32_t c0 = 0u, c1 = 0u;
    uint32_t ms_per_unit;
    uint32_t units;
    uint32_t spin;
    uint32_t loops = 0u;
    uint32_t elapsed_units;
    uint32_t elapsed_us;
    uint8_t  has_cycles;

    has_cycles = axk_bsp_cycles_supported();
    axk_bsp_cycles_enable();

    if (has_cycles) {
        uint32_t a = axk_bsp_cycles();
        axk_bsp_nop_loop(1000u);
        if (axk_bsp_cycles() == a) {
            has_cycles = 0u;            /* 计数器没接/没使能 */
        }
    }

    ms_per_unit = aiio_os_tick2ms(1u);
    if (ms_per_unit == 0u) {
        ms_per_unit = 1u;
    }

    units = (AXK_BSP_DELAY_CAL_MS + ms_per_unit - 1u) / ms_per_unit;
    if (units < 2u) {
        units = 2u;
    }

    if (aiio_os_tick_count(&t0) != 0) {
        axk_bsp_delay_fallback(has_cycles);
        return;
    }

    c0 = has_cycles ? axk_bsp_cycles() : 0u;
    t1 = t0;
    spin = 0u;

    while ((uint32_t)(t1 - t0) < units) {
        if (!has_cycles) {
            axk_bsp_nop_loop(1000u);
            loops += 1000u;
        }
        spin++;
        /* 有周期计数器时按“轮询次数”限幅，无计数器时按空转批次数限幅 */
        if (spin > (has_cycles ? 2000000u : 50000u)) {
            break;
        }
        if (aiio_os_tick_count(&t1) != 0) {
            break;
        }
    }

    c1 = has_cycles ? axk_bsp_cycles() : 0u;

    elapsed_units = (uint32_t)(t1 - t0);
    if (elapsed_units == 0u) {
        axk_bsp_delay_fallback(has_cycles);
        return;
    }

    elapsed_us = elapsed_units * ms_per_unit * 1000u;
    if (elapsed_us == 0u) {
        axk_bsp_delay_fallback(has_cycles);
        return;
    }

    if (has_cycles) {
        uint32_t cycles_per_us = (uint32_t)(c1 - c0) / elapsed_us;

        if (cycles_per_us >= 1u && cycles_per_us <= 20000u) {
            s_cycles_per_us = cycles_per_us;
            s_use_cycles    = 1u;
            s_delay_state   = 1u;
            return;
        }
    } else if (elapsed_us >= 1000u) {
        uint32_t loops_per_us = loops / elapsed_us;

        if (loops_per_us >= 1u && loops_per_us <= 10000u) {
            s_loops_per_us = loops_per_us;
            s_use_cycles   = 0u;
            s_delay_state  = 1u;
            return;
        }
    }

    axk_bsp_delay_fallback(has_cycles);
}

void axk_bsp_delay_init(void)
{
    if (s_delay_state == 0u) {
        axk_bsp_delay_calibrate();
    }
}

/* ── 延时主体 ─────────────────────────────────────────────────────── */

void axk_bsp_delay_us(uint32_t us)
{
    if (us == 0u) {
        return;
    }

    if (s_delay_state == 0u) {
        axk_bsp_delay_calibrate();
    }

    while (us > 0u) {
        uint32_t chunk = (us > AXK_BSP_DELAY_CHUNK_US) ? AXK_BSP_DELAY_CHUNK_US : us;

        if (s_use_cycles) {
            uint32_t start = axk_bsp_cycles();
            uint32_t limit = s_cycles_per_us * chunk;

            while ((uint32_t)(axk_bsp_cycles() - start) < limit) {
                /* busy-wait */
            }
        } else {
            axk_bsp_nop_loop(s_loops_per_us * chunk);
        }

        us -= chunk;
    }
}

void axk_bsp_delay_ms(uint32_t ms)
{
    while (ms >= 10u) {
        axk_bsp_delay_us(10000u);
        ms -= 10u;
    }
    if (ms > 0u) {
        axk_bsp_delay_us(ms * 1000u);
    }
}
