#!/usr/bin/env python3
"""Convert .config (kconfiglib output) to scbb_config.h.

The generated header has two compile-time modes:

  * open SDK build — `aiio_autoconf.h` is on the include path. The
    application's app_config/.config is the board definition, so its
    CONFIG_SCBB_*_ENABLED / CONFIG_AXK_* symbols are mapped in.
  * standalone build — no aiio_autoconf.h. The component's own .config
    (menuconfig.py) provides the values, as before.

Kconfig.scbb is the single source of truth for module/bus/pin names, so
the open-SDK branch is generated from it without a hardcoded module list.
"""

import re
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
SCBB_DIR = SCRIPT_DIR.parent
KCONFIG_SCBB = SCBB_DIR / "Kconfig.scbb"


def parse_config(config_path):
    result = {}
    with open(config_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" in line:
                key, val = line.split("=", 1)
                key = key.strip()
                val = val.strip()
                if key.startswith("CONFIG_"):
                    key = key[7:]
                if len(val) >= 2 and val[0] == '"' and val[-1] == '"':
                    val = val[1:-1]
                result[key] = val
    return result


def discover_modules(config):
    """Find all enabled modules and their BSP configs from .config."""
    modules = {}
    for key, val in config.items():
        if key.endswith("_ENABLED") and val == "y":
            # Strip SCBB_ prefix and _ENABLED suffix to get module name
            module = key
            if module.startswith("SCBB_"):
                module = module[5:]  # Remove SCBB_
            module = module.replace("_ENABLED", "")
            modules[module] = {}
    return modules


def discover_bsp_configs(config, module):
    """Find all HEADER and PREFIX configs for a module."""
    prefix = f"SCBB_{module}_"
    bsp_types = {}
    for key, val in config.items():
        if not key.startswith(prefix) or key.endswith("_ENABLED"):
            continue
        suffix = key[len(prefix):]
        if "_HEADER" in suffix:
            bsp_type = suffix.replace("_HEADER", "")
            if bsp_type not in bsp_types:
                bsp_types[bsp_type] = {"header": "", "prefix": ""}
            bsp_types[bsp_type]["header"] = val
        elif "_PREFIX" in suffix:
            bsp_type = suffix.replace("_PREFIX", "")
            if bsp_type not in bsp_types:
                bsp_types[bsp_type] = {"header": "", "prefix": ""}
            bsp_types[bsp_type]["prefix"] = val
    return bsp_types


def parse_kconfig_scbb(kconfig_path):
    """Extract module/bus/pin names and the axk defaults from Kconfig.scbb.

    The open SDK branch is generated from the Kconfig fragment so adding a
    module only requires touching Kconfig.scbb and its source files.
    """
    text = Path(kconfig_path).read_text(encoding="utf-8")
    modules = []
    pins = []
    axk_defaults = {}
    current = None

    for line in text.splitlines():
        m = re.match(r'^\s*config\s+(\w+)\s*$', line)
        if m:
            current = m.group(1)
            if current.startswith("SCBB_") and current.endswith("_ENABLED"):
                module = current[len("SCBB_"):-len("_ENABLED")]
                if module not in modules:
                    modules.append(module)
            elif current.startswith("AXK_"):
                pins.append(current)
            continue

        m = re.match(
            r'^\s*default\s+"([^"]+)"\s+if\s+SCBB_BSP_PLATFORM\s*=\s*"axk"\s*$',
            line,
        )
        if m and current:
            axk_defaults[current] = m.group(1)

    buses = {}
    for sym, val in axk_defaults.items():
        for module in sorted(modules, key=len, reverse=True):
            prefix = f"SCBB_{module}_"
            if not sym.startswith(prefix):
                continue
            rest = sym[len(prefix):]
            if rest.endswith("_HEADER"):
                bus, kind = rest[:-len("_HEADER")], "header"
            elif rest.endswith("_PREFIX"):
                bus, kind = rest[:-len("_PREFIX")], "prefix"
            else:
                break
            entry = buses.setdefault(module, {}).setdefault(bus, {})
            entry[kind] = val
            break

    return {"modules": modules, "pins": pins, "buses": buses}


def emit_pin_defines(pins, lines):
    """Emit #ifndef-guarded pin macros (manual overrides win)."""
    for name in sorted(pins):
        lines.append(f"#ifndef {name}")
        lines.append(f"#define {name} {pins[name]}")
        lines.append("#endif")


def emit_pins(config, lines):
    """Emit every AXK_* pin selected in menuconfig (standalone mode)."""
    lines.append("/* Pin assignment (menuconfig) */")

    pins = {
        key: val
        for key, val in config.items()
        if key.startswith("AXK_") and val not in ("", "n")
    }
    emit_pin_defines(pins, lines)
    lines.append("")


def emit_open_sdk_branch(info, lines):
    """Map the open SDK's CONFIG_* symbols into SCBB's AXK_* macros.

    aiio_autoconf.h is generated by `./build.sh <chip> <app> menuconfig`
    from the application's app_config/.config. No SDK build scripts are
    involved: this file is compiled with the SCBB sources.
    """
    lines.append("/* ── open SDK mode: application app_config/.config ──────────── */")
    lines.append("#ifndef SCBB_USE_BSP")
    lines.append("#define SCBB_USE_BSP 1")
    lines.append("#endif")
    lines.append("#ifndef SCBB_BSP_PLATFORM")
    lines.append('#define SCBB_BSP_PLATFORM "axk"')
    lines.append("#endif")
    lines.append("")

    if info["pins"]:
        lines.append("/* Pin assignment: CONFIG_AXK_* → AXK_* */")
        for pin in info["pins"]:
            lines.append(f"#ifdef CONFIG_{pin}")
            lines.append(f"#ifndef {pin}")
            lines.append(f"#define {pin} CONFIG_{pin}")
            lines.append("#endif")
            lines.append("#endif")
        lines.append("")

    lines.append("/* Module enables: CONFIG_SCBB_*_ENABLED → SCBB_*_ENABLED */")
    for module in info["modules"]:
        lines.append(f"#ifdef CONFIG_SCBB_{module}_ENABLED")
        lines.append(f"#define SCBB_{module}_ENABLED 1")
        lines.append("#endif")
    lines.append("")

    for module in info["modules"]:
        buses = info["buses"].get(module)
        if not buses:
            continue
        lines.append(f"/* {module} BSP routing (axk) */")
        lines.append(f"#ifdef SCBB_{module}_ENABLED")
        for bus in sorted(buses):
            entry = buses[bus]
            header = entry.get("header")
            prefix = entry.get("prefix")
            if not header or not prefix:
                continue

            lines.append(f"#ifndef SCBB_{module}_{bus}_HEADER")
            lines.append(f'#define SCBB_{module}_{bus}_HEADER "{header}"')
            lines.append("#endif")
            lines.append(f"#ifndef SCBB_{module}_{bus}_PREFIX")
            lines.append(f'#define SCBB_{module}_{bus}_PREFIX "{prefix}"')
            lines.append("#endif")

            if bus == "DELAY":
                lines.append(f"#ifndef AXK_{module}_DELAY_MS")
                lines.append(f"#define AXK_{module}_DELAY_MS(x) {prefix}_ms(x)")
                lines.append("#endif")
                lines.append(f"#ifndef AXK_{module}_DELAY_US")
                lines.append(f"#define AXK_{module}_DELAY_US(x) {prefix}_us(x)")
                lines.append("#endif")
            else:
                lines.append(f"#ifndef AXK_{module}_{bus}_ACLL")
                lines.append(
                    f"#define AXK_{module}_{bus}_ACLL(_func, ...) "
                    f"{prefix}##_##_func(__VA_ARGS__)"
                )
                lines.append("#endif")
        lines.append("#endif")
        lines.append("")


def emit_standalone_branch(config, lines):
    """Existing standalone (CMake + menuconfig.py) output."""
    emit_pins(config, lines)

    modules = discover_modules(config)

    # ── Built-in BSP selection ─────────────────────────────────────
    # CMakeLists.txt 通过 #define SCBB_USE_BSP / SCBB_BSP_PLATFORM "xxx"
    # 决定是否 add_subdirectory(BSP/<platform>/...) 以及链接哪些 BSP 目录。
    if config.get("SCBB_USE_BSP") == "y":
        lines.append("/* Built-in BSP selection */")
        lines.append("#define SCBB_USE_BSP 1")
        platform = config.get("SCBB_BSP_PLATFORM")
        if platform:
            lines.append(f'#define SCBB_BSP_PLATFORM "{platform}"')
        lines.append("")

    # ── Module enables ──────────────────────────────────────────────
    lines.append("/* Module enables */")
    for module in sorted(modules.keys()):
        lines.append(f"#define SCBB_{module}_ENABLED 1")
    lines.append("")

    # ── Per-module BSP configs ─────────────────────────────────────
    for module in sorted(modules.keys()):
        bsp_configs = discover_bsp_configs(config, module)
        if not bsp_configs:
            continue

        lines.append(f"/* {module} BSP configuration */")
        for bsp_type in sorted(bsp_configs.keys()):
            info = bsp_configs[bsp_type]
            header = info["header"]
            prefix = info["prefix"]

            if header:
                define_name = f"SCBB_{module}_{bsp_type}_HEADER"
                lines.append(f'#define {define_name} "{header}"')

            if bsp_type != "DELAY":
                macro_name = f"AXK_{module}_{bsp_type}_ACLL"
                lines.append(
                    f"#define {macro_name}(_func, ...) "
                    + prefix + "##_##_func(__VA_ARGS__)"
                )

            if bsp_type == "DELAY":
                macro_name = f"AXK_{module}_DELAY_MS"
                lines.append(
                    f"#define {macro_name}(x) " + prefix + "_ms(x)"
                )
                macro_name = f"AXK_{module}_DELAY_US"
                lines.append(
                    f"#define {macro_name}(x) " + prefix + "_us(x)"
                )

        lines.append("")


def generate_header(config, output_path, kconfig_info):
    lines = [
        "/*",
        " * Auto-generated by scripts/gen_config_header.py — do not edit manually.",
        " *",
        " * Two modes, chosen at compile time:",
        " *   open SDK   — aiio_autoconf.h is on the include path; the app's",
        " *                app_config/.config provides CONFIG_SCBB_* / CONFIG_AXK_*.",
        " *   standalone — no aiio_autoconf.h; the component's own .config",
        " *                (menuconfig.py) provides the values below.",
        " */",
        "",
        "#ifndef SCBB_CONFIG_H",
        "#define SCBB_CONFIG_H",
        "",
        "/* 板级引脚表：用户手改的引脚覆盖，必须先于各模块的默认值 */",
        "#if defined(__has_include)",
        "#if __has_include(\"scbb_board_config.h\")",
        "#include \"scbb_board_config.h\"",
        "#endif",
        "#endif",
        "",
        "#if defined(__has_include)",
        "#if __has_include(\"aiio_autoconf.h\")",
        "#include \"aiio_autoconf.h\"",
        "#define SCBB_CONFIG_FROM_OPEN_SDK 1",
        "#endif",
        "#endif",
        "",
        "#ifdef SCBB_CONFIG_FROM_OPEN_SDK",
    ]
    emit_open_sdk_branch(kconfig_info, lines)
    lines.append("#else /* standalone build */")
    emit_standalone_branch(config, lines)
    lines.append("#endif /* SCBB_CONFIG_FROM_OPEN_SDK */")
    lines.append("")
    lines.append("#endif /* SCBB_CONFIG_H */")
    lines.append("")
    Path(output_path).write_text("\n".join(lines), encoding="utf-8")


def main():
    config_path = sys.argv[1] if len(sys.argv) > 1 else ".config"
    output_path = sys.argv[2] if len(sys.argv) > 2 else "scbb_config.h"

    if not Path(config_path).exists():
        print(f"Error: {config_path} not found. Run 'python3 menuconfig.py' first.", file=sys.stderr)
        sys.exit(1)

    config = parse_config(config_path)
    kconfig_info = parse_kconfig_scbb(KCONFIG_SCBB)
    generate_header(config, output_path, kconfig_info)
    print(f"Generated {output_path}")


if __name__ == "__main__":
    main()
