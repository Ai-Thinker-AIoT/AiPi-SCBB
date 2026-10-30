# BSP/axk —— 基于 open SDK 中间件的通用 BSP

`axk` 平台是 AiPi-SCBB 在 **aithinker_dev_open_sdk（aiio 中间件）**上的
BSP 实现，分两层：

1. **模块契约层**：`axk_bsp_gpio_*` / `axk_bsp_i2c_*` / `axk_bsp_spi_*` /
   `axk_bsp_uart_*` / `axk_bsp_delay_*`，与 `BSP/Ai-M6x`、`BSP/stm32f10x`
   的同名 BSP 一一对应，模块通过 ACLL 宏调用，SCBB 一行代码都不用改。
2. **外设封装层**：把 open SDK `aiio_*` 的全部外设 API 都包了一层
   `axk_bsp_<外设>_*`，包括 ADC、PWM、PWM 捕获、硬件 I2C/SPI、I2S、IR、
   RTC、WDT、Flash、NVS、低功耗、系统接口。应用可以直接用这套封装，
   不必再面向 `aiio_*` 编程。

目标：**以后新增任何芯片，只要 open SDK 里有它的 port，SCBB 不需要改一行代码。**

---

## 1. 支持的芯片

| open SDK chip | 内核 | GPIO | UART | 备注 |
|---|---|---|---|---|
| bl602 | RISC-V | aiio_hal_gpio | UART1 = 4/5 | 无 cycle CSR，延时用空转标定 |
| bl616 / bl616_iot / bl616cl / bl618 | RISC-V | aiio_hal_gpio | UART1 = 16/17 | bl616cl 重复 init 返回错误（本 BSP 忽略） |
| bs21 | ARM Cortex-M4 | aiio_hal_gpio | UART1 = 19/20 | 必须先 gpio_init 才能设方向 |
| ln882h | ARM Cortex-M4 | aiio_hal_gpio，pin 是 `GPIO_PIN_n` 位掩码 | UART1 = PA2/PA3 | recv 是阻塞的 |
| rtl8711dx | ARM Cortex-M4 | 固定引脚表，不支持的脚会空指针 | TX=PB5/PB31，RX=PB4/PB30 | recv 是阻塞的 |
| rtl8711fx | ARM Cortex-M4 | 有 C 口 | UART1 = PA26/PA25 | recv 是阻塞的 |
| rtl8720dx | ARM Cortex-M4 | A/B 口 | UART1 = PB1/PB2 | recv 是阻塞的 |

> LN882H 的 `AIIO_GPIO_NUM_n` 是位掩码，axk 已在
> `axk_bsp_gpio_num_of()` 里用 `__AIIO_GPIO_NUM_n` 逐项映射，模块侧不用管。

### 1.1 外设封装覆盖

| 外设 | bl602 | bl616 | bl616cl | bl616_iot | bl618 | bs21 | ln882h | rtl8711dx | rtl8711fx | rtl8720dx |
|---|---|---|---|---|---|---|---|---|---|---|
| GPIO / GPIO 中断 | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ |
| ADC | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ |
| PWM | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ |
| PWM 捕获 | ✔ | — | — | — | — | — | ✔ | — | — | — |
| 硬件 I2C | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ |
| 硬件 SPI | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ |
| I2S | — | ✔ | ✔ | — | ✔ | — | — | — | — | — |
| IR | ✔ | ✔ | ✔ | — | ✔ | — | — | — | — | — |
| RTC | ✔ | ✔ | ✔ | ✔ | ✔ | — | — | ✔ | ✔ | ✔ |
| WDT | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | — | ✔ | ✔ | ✔ |
| Flash | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ | ✔ |
| NVS | ✔ | ✔ | ✔ | ✔ | ✔ | — | ✔ | ✔ | ✔ | ✔ |
| 低功耗 | ✔ | ✔ | ✔ | ✔ | ✔ | — | ✔ | ✔ | ✔ | ✔ |
| 复位 / 随机数 | ✔ | ✔ | ✔ | ✔ | ✔ | 复位 — | ✔ | ✔ | ✔ | ✔ |
| RD-01 雷达 | ✔* | — | — | — | — | — | — | — | — | — |

“—”= 该芯片 port 没有实现，axk 在**运行期**返回
`AXK_BSP_ERR_UNSUPPORTED`（不会链接失败，机制见第 4 节）。
表中是中间件头文件层面的覆盖，个别函数的实现差异见第 9 节。

`*` RD-01 雷达在 bl602 上还要求应用配置 `CONFIG_AIIO_RADAR=y`，
axk 封装本身不包含 `aiio_radar.h`，见 [8.15](#815-rd-01-雷达radaraxk_bsp_radarh)。

---

## 2. 目录结构

```
BSP/axk/
├── axk_bsp_config.h      # 唯一配置入口：引脚编码、总线引脚、UART 固定口
├── README.md
│
│  ── SCBB 模块契约层 ──────────────────────────────────
├── gpio/                 # axk_bsp_gpio_*    → aipi_bsp_gpio
├── delay/                # axk_bsp_delay_*   → aipi_bsp_delay
├── i2c/                  # 事务式 + 软时序 I2C → aipi_bsp_i2c
├── spi/                  # 软件 SPI（模式 0~3）→ aipi_bsp_spi
├── uart/                 # 带名 UART + 固定口 UART → aipi_bsp_uart
│
│  ── open SDK 外设封装层 ──────────────────────────────
├── adc/                  # axk_bsp_adc_*         → aipi_bsp_adc
├── flash/                # axk_bsp_flash_*       → aipi_bsp_flash
├── i2s/                  # axk_bsp_i2s_*         → aipi_bsp_i2s
├── ir/                   # axk_bsp_ir_*          → aipi_bsp_ir
├── nvs/                  # axk_bsp_nvs_*         → aipi_bsp_nvs
├── pwm/                  # axk_bsp_pwm_*         → aipi_bsp_pwm
├── pwmcap/               # axk_bsp_pwmcap_*      → aipi_bsp_pwmcap
├── radar/                # axk_bsp_radar_*       → aipi_bsp_radar（RD-01）
├── rtc/                  # axk_bsp_rtc_*         → aipi_bsp_rtc
├── sleep/                # axk_bsp_sleep_*       → aipi_bsp_sleep
├── system/               # axk_bsp_system_*      → aipi_bsp_system
└── wdt/                  # axk_bsp_wdt_*         → aipi_bsp_wdt
```

每个子目录的 `CMakeLists.txt` 会创建 `aipi_bsp_<协议>` 静态库，并把
上一级目录加入 include 路径（给 `axk_bsp_config.h` 用）。i2c / spi / uart
还会自动补上自己依赖的 gpio / delay 目标，避免模块组合不同导致漏链。
`SCBB_BSP_PLATFORM="axk"` 时根 `CMakeLists.txt` 会把上面**所有**目录都
`add_subdirectory` 并挂到 `aipi_scbb` 上，应用按需调用即可。

---

## 3. 在 SCBB 里启用 axk

### 3.1 生成配置头

```bash
cd AiPi-SCBB
python scripts/gen_config_header.py scripts/axk.config scbb_config.h
```

`scripts/axk.config` 已把 10 个可移植模块（除 WS2812、ST7789V_LCD）全部
打开，并按 axk 配好头文件/前缀，同时写入：

```
CONFIG_SCBB_USE_BSP=y
CONFIG_SCBB_BSP_PLATFORM="axk"
```

生成出的 `scbb_config.h` 会包含：

```c
#define SCBB_USE_BSP 1
#define SCBB_BSP_PLATFORM "axk"
...
#define SCBB_DHT11_GPIO_HEADER "axk_bsp_gpio.h"
#define AXK_DHT11_GPIO_ACLL(_func, ...) axk_bsp_gpio##_##_func(__VA_ARGS__)
```

### 3.2 CMake

根 `CMakeLists.txt` 会读取 `scbb_config.h` 里的 `SCBB_BSP_PLATFORM`，自动
`add_subdirectory(BSP/axk/<协议>)` 并链接需要的 `aipi_bsp_*`，无需额外参数。

### 3.3 open SDK 组件（flat-copy）

open SDK 的 `src_memcpy` 会把组件目录里的 **所有 .c/.h 扁平拷贝**进
`dev_project/src` 一起编译。因此组件副本里**只能保留一个 BSP 平台目录**：

- 用 axk：组件里只留 `BSP/axk/`，删掉 `BSP/Ai-M6x/`、`BSP/stm32f10x/`
- 用 Ai-M6x：组件里只留 `BSP/Ai-M6x/`（同样删掉其它平台目录）

否则两套 BSP 会一起编译，非目标平台的源码会因找不到 SDK 头文件而报错。
`axk_bsp_*` 与 `bsp_*` / `stm32f10x_*` 名字不同，不会符号冲突，但编译不过。

---

## 4. 弱符号机制与返回码

open SDK 的 flat-copy 构建会编译组件里的**每一个** `.c`，而不同芯片的
`port/` 实现并不齐全（例如 I2S 只有 BL616 系有）。如果封装直接调用
`aiio_i2s_init()`，在 bs21 上就会链接失败。

所以**外设封装层**统一用弱引用：

```c
extern int aiio_i2s_init(aiio_i2s_config_t cfg) AXK_BSP_WEAK;

if (!AXK_BSP_WEAK_AVAILABLE(aiio_i2s_init)) {
    return AXK_BSP_ERR_UNSUPPORTED;
}
```

- GCC/Clang：`__attribute__((weak))`，未定义的弱符号解析为地址 0
- `AXK_BSP_WEAK_AVAILABLE(fn)` 展开为 `((fn) != NULL)`

这样同一份 axk 代码可以在全部芯片上编译链接，调用不支持的外设时安全地
返回错误码，而不是崩溃或链接失败。

返回码定义在 `axk_bsp_config.h`：

| 宏 | 值 | 含义 |
|---|---|---|
| `AXK_BSP_OK` | 0 | 成功 |
| `AXK_BSP_ERR` | -1 | 失败（中间件返回了错误） |
| `AXK_BSP_ERR_PARAM` | -2 | 参数错误（空指针、通道号越界等） |
| `AXK_BSP_ERR_UNSUPPORTED` | -3 | 当前芯片没有实现该 API |

少数直接透传中间件状态的函数（NVS / Flash / Sleep）返回值语义见第 8 节。

---

## 5. 引脚编码（重点）

### 5.1 GPIO 引脚：`0x100 | n` 表示端口

模块只传一个 `uint16_t`，而 `aiio_hal_gpio_*` 需要 `(port, pin)`。
axk 约定：

| 编码 | 含义 |
|---|---|
| `0x00 ~ 0xFF` | 端口 A，引脚号为低 8 位 |
| `0x100 \| n` | 端口 B，引脚 n |
| `0x200 \| n` | 端口 C，引脚 n（仅 rtl8711fx） |

也提供宏：`AXK_BSP_GPIO_PIN(1, 31)` == `0x11F`。

所以：

- BL 系列写裸号即可（IO18 → `18`，IO40 → `40`）
- RTL 的 PB31 → `AXK_BSP_GPIO_PIN(1, 31)`
- rtl8711fx 的 PC5 → `AXK_BSP_GPIO_PIN(2, 5)`

ADC / PWM / PWM 捕获 / I2S / IR 的引脚参数同样走这套编码，封装内部再拆成
`(port, pin)`。

### 5.2 UART 引脚：中间件原生 pin 值

`aiio_uart_config_t` **没有 port 字段**，端口由各芯片的 UART 实现按 pin
号查表决定。因此 `AXK_BSP_UARTx_TX/RX_PIN` 必须直接写中间件原生值
（如 `AIIO_GPIO_NUM_5`、`12`、LN882H 的 `GPIO_PIN_2`），
**不要用 `AXK_BSP_GPIO_PIN()` 编码**。

---

## 6. 模块引脚与 BSP 引脚必须一致

SCBB 的 BSP 接口是 `init(name, cfg)` 形式，**拿不到模块的引脚宏**。
所以模块自己的引脚宏要和 BSP 配置对应：

| 模块 | 模块侧宏 | BSP 侧使用 |
|---|---|---|
| DHT11 | `AXK_DHT11_PIN` | 直接作为逻辑引脚号 |
| RELAY | `AXK_RELAY_PIN` | 同上 |
| DS1302 | `AXK_DS1302_CE/SCLK/IO_PIN` | 同上 |
| OLED_096_SPI | `AXK_OLED_096_SPI_SCL/SDA/CS/DC/RST_PIN` | SPI 用 `spi0`（默认 13/11/11），其余 GPIO 用模块自己的宏 |
| LLCC68 | `AXK_LLCC68_*_PIN`、`AXK_LLCC68_SPI_NAME` | SPI 用 `spi1`（默认 5/19/19） |
| RD03_V2 | `AXK_RD03_V2_UART`（名字） | UART 引脚取 `AXK_BSP_UARTx_TX/RX_PIN` |
| HXD039B2 | BUSY 引脚 | UART 固定口 `AXK_BSP_UART_FIXED_DEV` |
| INA226 | `AXK_INA226_I2C`（名字） | I2C 引脚取 `AXK_BSP_I2C_SCL/SDA_PIN` |
| SHT3x / CH224A | 无引脚宏 | 软 I2C 引脚取 `AXK_BSP_I2C_SOFT_SCL/SDA_PIN` |

所有 BSP 侧引脚都在 `axk_bsp_config.h` 中，带 `#ifndef`，可以在
`scbb_config.h` 里覆盖。

---

## 7. 延时与 `axk_bsp_delay_init()`

axk 的延时全部是**忙等**，不调用 `aiio_os_tick_dealy()`（后者在临界区里
不会让出 CPU，行为不可控），因此可以在 `taskENTER_CRITICAL()` 和中断里
安全使用。

us 级精度来自“现场标定”：

- Cortex-M3/M4/M7：DWT->CYCCNT（`__ARM_ARCH >= 7` 时默认启用）
- RISC-V：**默认不启用周期计数器**。BL602 的核没有实现 cycle/mcycle CSR，
  执行 `rdcycle` 会触发非法指令异常，SDK 默认 `handle_trap` 又是死循环，
  表现就是程序在第一次延时标定时“跑飞”、串口无任何报错。
  确认目标核支持时可 `-DAXK_BSP_DELAY_USE_CYCLES=1` 打开。
- 标定基准：`aiio_os_tick_count()` + `aiio_os_tick2ms()`
- 计数器不存在 / 标定失败：退回空转循环估算，或 `AXK_BSP_CPU_MHZ`

**建议在上电初始化、进入第一个临界区之前调用一次：**

```c
#include "axk_bsp_delay.h"

void app_init(void)
{
    axk_bsp_delay_init();      /* 此时 tick 正常走动，标定最准 */
    /* ... 之后再进 taskENTER_CRITICAL() 读 DHT11 ... */
}
```

不调用也能工作：第一次 `axk_bsp_delay_us/ms` 会懒标定。但如果第一次延时
就发生在临界区里（tick 冻结），标定会失败并退到估算值，30us 级别的
DHT11 时序可能不准。

可覆盖参数（写在 `scbb_config.h`）：

| 宏 | 默认 | 说明 |
|---|---|---|
| `AXK_BSP_DELAY_CAL_MS` | 20 | 标定窗口（ms） |
| `AXK_BSP_CPU_MHZ` | 320 | 标定失败时的兜底主频 |
| `AXK_BSP_DELAY_FALLBACK_LOOPS_PER_US` | 64 | 无周期计数器时的兜底空转次数/us |

---

## 8. 外设封装 API 一览

> 这些是**额外**于 SCBB 模块契约层的能力，模块本身不用它们也可以。
> 头文件都在对应子目录下，例如 `adc/axk_bsp_adc.h`。

### 8.1 GPIO（gpio/axk_bsp_gpio.h）

`init` / `set` / `reset` / `read` / `deinit` /
`int_register` / `int_enable`

- 回调类型 `axk_bsp_gpio_irq_cb_t(int32_t irq, void *param)`
- **BL 系列触发边沿由上下拉推导**（下拉 → 上升沿，上拉 → 下降沿），
  注册前先用 `init(pin, AXK_BSP_GPIO_MODE_IN_PU / _IN)` 设好上下拉

### 8.2 ADC（adc/axk_bsp_adc.h）

`axk_bsp_adc_init(cfg)` / `axk_bsp_adc_read(channel)`

- `cfg = {channel, mode(单次/连续), resolution(8~16), vref}`
- 返回值单位是 **毫伏（mV）**，不是原始码
- bl616cl 一次只能配置一个通道，切换通道要重新 init

### 8.3 PWM（pwm/axk_bsp_pwm.h）

`init(freq_hz, duty_pct, pin)` → 通道号 / `start` / `set_duty` /
`get_duty` / `set_freq` / `get_freq` / `output_close` / `output`

- 频率单位统一为 **Hz**
- 中间件 `aiio_pwm_set_freq()` 头文件写的是微秒，但 BL 实现按 Hz 处理，
  本封装按实现命名为 Hz
- bl616cl 的 `aiio_pwm_output()` 是空实现（返回错误），`get_duty()`
  出错时返回 `-1.0f`，封装转成 `AXK_BSP_ERR`

### 8.4 PWM 捕获（pwmcap/axk_bsp_pwmcap.h）

`init(cfg{pin})` / `start` / `close` / `read(freqlist, dutylist, size)`

- 只有 **bl602 / ln882h** 实现

### 8.5 硬件 I2C（i2c/axk_bsp_i2c_hw.h）

`init(cfg{port, sda, scl, freq_hz})` / `deinit` / `write` / `read` /
`write_byte` / `read_byte`

- `dev_addr` 传 **7 位地址**，封装内部左移 1 位
- `freq_hz` 最终作为 I2C 时钟频率传给 `bflb_i2c_init()`
- bl616cl 只支持 `port = 0`；每次写内部固定延时 100ms
- bl616cl 的 `aiio_i2c_master_write_byte()` 有指针 bug，本封装的
  `write_byte` 改走 `write(..., len=1)` 规避

### 8.6 硬件 SPI（spi/axk_bsp_spi_hw.h）

`init(cfg{port, clk, mosi, miso, cs})` / `deinit` / `write` / `read` /
`write_byte` / `read_byte` / `write_and_read`

- bl616cl 只支持 `port = 0`，固定 1MHz / 模式 0 / 8 位 / MSB first，
  CS 由硬件控制，`timeout` 参数被忽略
- 需要自定义速率/模式的芯片请查各自 port 实现

### 8.7 I2S（i2s/axk_bsp_i2s.h）

`init(cfg{port, fs, di, do_, bclk, mclk})` / `deinit`

- 只有 **bl616 / bl616cl / bl618**
- 中间件只提供 init/deinit；BL 实现固定 16kHz / 16bit，内部 DMA
  写入自己的 `rx_buffer`，MCLK 走 CLKOUT
- `do_` 是数据输出脚，避免 C++ 关键字

### 8.8 IR（ir/axk_bsp_ir.h）

`init(pin, cfg)` / `enable` / `receive(&data, &bit_cnt)`

- 只有 **bl602 / bl616 / bl616cl / bl618**
- BL 实现只使用 `mode` / `input_inverse` / `deglitch_enable`，三个
  阈值参数被忽略

### 8.9 RTC（rtc/axk_bsp_rtc.h）

`init` / `deinit` / `get_time(&timestamp_s)` / `set_time(timestamp_s)`

- 单位是**秒**（Unix 时间戳）
- bs21 / ln882h 没有实现
- BL 的 `set_time` 只保存一个偏移量，不是写硬件 RTC

### 8.10 WDT（wdt/axk_bsp_wdt.h）

`init` / `init_timeouts(ms)` / `deinit` / `reload`

- ln882h 没有实现
- 重复 `init` 被中间件忽略；BL 把超时值当比较值使用

### 8.11 Flash（flash/axk_bsp_flash.h）

`init` / `deinit` / `write` / `write_erase` / `read` / `erase` /
`open` / `info_get` / `close` /
`id_read` / `device_id_read` / `unique_id_read` / `manufacturer_id_read` /
`status_read` / `status_register1_read` / `status_register2_read`

- `open()` 返回 **中间件原始值**：0 表示失败，非 0 是句柄/设备指针
- `info_get()` / `close()` 把句柄按 `(flash_dev_t *)(intptr_t)handle`
  还原，所以 handle 必须原样传回
- 只有 bl602 / ln882h 实现了 open/info_get/close 和完整的 ID/状态读取；
  其它芯片调用返回 `AXK_BSP_ERR_UNSUPPORTED`
- bl602 的 `write_erase()` 忽略 addr，固定擦写 “DATA” 分区

### 8.12 NVS（nvs/axk_bsp_nvs.h）

`init` / `deinit` / `erase_key` / `erase_all` / `dump` /
`get_blob` / `set_blob`

- bs21 没有实现
- 状态类函数返回 `int`：0 成功，>0 是中间件 NVS 错误码，<0 是 axk 错误码
- `get_blob()` 返回实际长度，0 表示键不存在或不支持

### 8.13 低功耗（sleep/axk_bsp_sleep.h）

`axk_bsp_sleep_enter(cfg)`

- `cfg = {mode, wake_source, wake_ms, wake_pin, wake_level}`
- bs21 没有实现
- `wake_pin` 是**中间件映射后的引脚序号**，不是 axk 编码
- bl616cl 只支持深睡眠 + 定时唤醒，GPIO 唤醒在实现里被关闭
- 返回码沿用中间件：0 成功 / 1 失败 / 2 模式不支持 / 3 参数错误 /
  4 引脚不支持唤醒 / 128 未适配

### 8.14 系统（system/axk_bsp_system.h）

`restart` / `random` / `chip_name` / `chip_init`

- bs21 没有 `aiio_restart`
- `random` 未实现时返回 0
- `chip_init` 应用 `main()` 已经调用过，业务代码不要重复调用

### 8.15 RD-01 雷达（radar/axk_bsp_radar.h）

`init` / `data_cb_register` / `ack_cb_register` / `cmd_process`

- 只有 **bl602** 实现，且要求 `CONFIG_AIIO_RADAR=y`；其它情况返回
  `AXK_BSP_ERR_UNSUPPORTED`
- 本封装**不包含** `aiio_radar.h`（该头依赖应用生成的 `aiio_autoconf.h`，
  且只在 `CONFIG_AIIO_RADAR` 打开时声明 API），而是用弱引用 + 自定义
  回调类型，保证所有芯片都能编译
- 回调类型 `void (*)(uint8_t *data, uint16_t len)`

---

## 9. 芯片差异与已知限制

### 9.1 GPIO

- **bl616cl**：已初始化引脚再次 `init` 会返回错误；`direction_set` /
  `pull_set` 内部会 deinit+init；`PULL_NONE` 实际被当成上拉。axk 忽略
  init 返回值，按“init → direction → pull”顺序调用，行为正确。
- **ln882h**：`init` 总是先配成输出，必须再调 `direction_set(INPUT)`；
  axk 已这样做。引脚号是位掩码，已由映射函数处理。
- **bs21**：`direction_set` / `pull_set` 要求先 `init`；axk 已保证顺序。
- **rtl8711dx**：GPIO 用固定引脚表，**不支持的引脚会在中间件里解引用空
  指针**。请使用该芯片 port 支持的引脚；例如软 I2C 默认的 PA8/PA18 可用，
  PA4/PA5 不可用（用不了就把软 I2C 改到支持的表项引脚）。
- **rtl8711fx**：唯一有 C 口的芯片。

### 9.2 UART

- `aiio_uart_recv_data()` 的语义不统一：
  - BL602/BL616/BL616CL/BL618：非阻塞，返回实际读到的字节数（无数据为 0）
  - BS21：`usr_uart_read_data(..., 0)`，非阻塞
  - LN882H：**阻塞**，永远返回 1
  - RTL8711DX/FX/RTL8720DX：**阻塞**，每次读 1 字节

  `axk_bsp_uart_getchar()` 统一返回 `-1` 表示无数据，但在 LN882H/RTL 上
  无数据时不会返回，而是一直阻塞。RD03_V2 的“100 次超时”在这些芯片上
  会退化成永久阻塞；如果应用不能接受，需换用中断接收或选 BL/BS21。

- RTL8711DX 只接受 TX=PB5/PB31、RX=PB4/PB30；RTL8711FX 只接受
  TX=PA26/RX=PA25；RTL8720DX 只接受 TX=PB1/PB2。其它脚位中间件直接
  返回 `AIIO_UART_PIN_NOT_SUPPORT`。
- AI-Thinker 的 AT 固件默认占用 UART1（各芯片脚位见上表），用 RD03/HXD
  时请选一个应用没占用的串口。

### 9.3 不支持 axk 的模块

| 模块 | 原因 |
|---|---|
| **WS2812** | 需要 PWM+DMA 产生亚微秒级归零码，软件 GPIO 无法稳定实现 |
| **ST7789V_LCD** | 模块 `.c` 直接 `#include "bflb_gpio.h"` / `"lcd.h"`，与 open SDK 无关的 BL 专有代码，需模块自身重构后才能用 |

其余 10 个模块均可用 axk。

---

## 10. 不在 BSP 范围内

以下 `aiio_*` API 属于**无线/协议栈/平台服务**，不是“外设”，axk 不封装，
应用直接调用中间件即可：

- `aiio_wifi_*`、`aiio_ble_*`、`aiio_ali_ble_*`
- `aiio_mdns_*`、`aiio_sntp_*`、`aiio_platform_ota_*`
- `aiio_uart_log_*`（日志）、`aiio_at_get_gpio_by_map()`（AT 平台）

硬件外设（含 RD-01 雷达）已全部封装，见第 8 节。

---

## 11. 快速验证

以 open SDK 的 bl602 为例。SCBB **不改 open SDK 的构建脚本**，应用侧接入只有
三件事（`build.sh` / `os_make.sh` / `adapter_modules` 都保持原样）。

SCBB 自带的脚本能把下面 1、2 一次做完，并直接打开 SDK 的 menuconfig
（它只改应用侧文件，幂等，可重复执行）：

```bash
python3 adt/adt_scbb/menuconfig.py --open-sdk <chip> <app>
# 不记得 chip/app：先弹列表选，再进 SDK 菜单
python3 adt/adt_scbb/menuconfig.py --open-sdk
# 只挂接、不打开菜单：
python3 adt/adt_scbb/menuconfig.py --open-sdk <chip> <app> --attach-only
```

省略 `<chip>` / `<app>` 时，脚本会从 `application/*/vendor/*/app_config/`
里列出可选芯片，再按芯片列出应用，选完自动进入 SDK 菜单。退出菜单后
脚本会打印下一步的 `./build.sh <chip> <app> cn debug`。下面的 1–3 是
等价的纯手工步骤，便于理解或排查。

1. 把 SCBB 放到 `adt/adt_scbb`，由脚本在应用 `src/` 下生成一个"源码视图"
   `adt_scbb/`：真实目录，里面只有指向 `BSP/axk`、各模块目录和 `scbb_*.h`
   的符号链接。SDK 现有的 `src_memcpy` 会递归平铺视图里的 `.c/.h`，所以
   `BSP/Ai-M6x`、`BSP/stm32f10x` 虽然还在仓库里，但构建永远看不到它们。
   手工等价操作：

   ```bash
   mkdir -p application/<app>/src/adt_scbb/BSP
   ln -s ../../../../../adt/adt_scbb/BSP/axk application/<app>/src/adt_scbb/BSP/axk
   for d in CH224A DHT11 DS1302 HXD039B2 INA226 LLCC68 OLED_096_SPI \
            RD03_V2 RELAY SHT3x ST7789V_LCD WS2812; do
       ln -s ../../../../adt/adt_scbb/$d application/<app>/src/adt_scbb/$d
   done
   ln -s ../../../../adt/adt_scbb/scbb_config.h application/<app>/src/adt_scbb/
   ln -s ../../../../adt/adt_scbb/scbb_board_config.h application/<app>/src/adt_scbb/
   ```

   推荐直接用 `menuconfig.py --open-sdk <chip> <app> --attach-only` 自动生成。

2. 在应用自己的 `vendor/<chip>/app_config/Kconfig` 里 source SCBB 的片段。
   这个文件属于应用/板级配置，不是 SDK：

   ```kconfig
   config SCBB_BSP_PLATFORM
       string
       default "axk"

   source "../../../adt/adt_scbb/Kconfig.scbb"
   ```

3. 用 SDK 的 menuconfig 选模块/引脚，值写进应用 `.config` 的
   `CONFIG_SCBB_*` / `CONFIG_AXK_*`，然后编译：

   ```bash
   ./build.sh bl602 <app> menuconfig
   #    AiPi-SCBB → 选模块
   #    AiPi-SCBB → Pin assignment → 填 CONFIG_AXK_* 引脚
   ./build.sh bl602 <app> cn debug
   ```

> 第一次必须先跑一次 `menuconfig`（哪怕不改任何选项）。除了生成应用
> `.config`，它还会把 `aiio_autoconf.h` 放到
> `tmp/kconfig_h/<chip>/<app>/`；直接跳到 `cn debug` 会报
> `error: no ./build.sh $chip $application menuconfig`。

编译时 `scbb_config.h` 发现 include 路径上的 `aiio_autoconf.h`，就把
`CONFIG_SCBB_*_ENABLED` / `CONFIG_AXK_*` 映射成 `SCBB_*` / `AXK_*`，
模块驱动和 axk BSP 都从这里拿配置；没有 `aiio_autoconf.h` 时走 standalone 分支。

应用入口记得先调用 `axk_bsp_delay_init()`（见第 7 节）。

> standalone（CMake）构建仍然用 `python3 menuconfig.py` 或
> `python3 scripts/gen_config_header.py scripts/axk.config scbb_config.h`；
> 手改 `scbb_board_config.h` 在两种构建里都优先于 menuconfig 的值。

最小 GPIO 自测（不接外设，用万用表/示波器看电平翻转）：

```c
#include "axk_bsp_gpio.h"
#include "axk_bsp_delay.h"

axk_bsp_delay_init();
axk_bsp_gpio_init(AXK_BSP_GPIO_PIN(0, 0), AXK_BSP_GPIO_MODE_OUT);
for (;;) {
    axk_bsp_gpio_set(0);
    axk_bsp_delay_ms(500);
    axk_bsp_gpio_reset(0);
    axk_bsp_delay_ms(500);
}
```

外设封装层的最小自测（PWM 呼吸 + ADC 回读）：

```c
#include "axk_bsp_adc.h"
#include "axk_bsp_pwm.h"

axk_bsp_adc_cfg_t adc = {
    .channel = AXK_BSP_ADC_CH0, .mode = AXK_BSP_ADC_MODE_ONE_SHOT,
    .resolution = AXK_BSP_ADC_RES_12, .vref = AXK_BSP_ADC_VREF_3V2,
};
axk_bsp_adc_init(&adc);

int ch = axk_bsp_pwm_init(1000, 50.0f, AXK_BSP_GPIO_PIN(0, 0));
axk_bsp_pwm_start(ch);
int mv = axk_bsp_adc_read(AXK_BSP_ADC_CH0);   /* 单位 mV */
```
