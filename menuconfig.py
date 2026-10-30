#!/usr/bin/env python3
"""AiPi-SCBB menuconfig — standalone Kconfiglib menu + open SDK helper.

Usage:
    python menuconfig.py
        Standalone: edit SCBB's .config and generate scbb_config.h.

    python menuconfig.py --open-sdk [<chip> [<app>]] [--attach-only]
        Attach SCBB to application/<app> and run the open SDK's
        ./build.sh <chip> <app> menuconfig (which already contains the
        AiPi-SCBB submenu).  Omit chip/app to pick them from a list.
        --attach-only just creates the source view and the Kconfig source line.
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

# Set console window title
sys.stdout.write("\033]0;AiPi-SCBB\007")
sys.stdout.flush()

ROOT = Path(__file__).resolve().parent
SDK_ROOT = ROOT.parent.parent
CONFIG_FILE = ".config"
CONFIG_HEADER_NAME = "scbb_config.h"


def find_menuconfig():
    """Find menuconfig executable.

    Windows virtualenvs ship Scripts/menuconfig.exe. On Linux/macOS kconfiglib is
    a single module (no __main__), so `python -m kconfiglib menuconfig` does
    nothing; use the menuconfig.py installed next to kconfiglib.py instead.
    """
    scripts_dir = Path(sys.executable).parent / "Scripts"
    menuconfig_exe = scripts_dir / "menuconfig.exe"
    if menuconfig_exe.exists():
        return [str(menuconfig_exe), "Kconfig"]

    try:
        import kconfiglib
        installed = Path(kconfiglib.__file__).resolve().with_name("menuconfig.py")
        if installed.exists():
            return [sys.executable, str(installed), "Kconfig"]
    except ImportError:
        pass

    found = shutil.which("menuconfig")
    if found:
        return [found, "Kconfig"]

    return [sys.executable, "-m", "kconfiglib", "menuconfig", "Kconfig"]


def find_and_remove_old_config_headers():
    """Find and remove all existing scbb_config.h files to ensure uniqueness."""
    removed = []
    for f in ROOT.rglob(CONFIG_HEADER_NAME):
        # Skip .git and .mimocode directories
        parts = f.relative_to(ROOT).parts
        if any(p.startswith(".") for p in parts):
            continue
        f.unlink()
        removed.append(str(f.relative_to(ROOT)))
    return removed


def discover_chips():
    """List chips that at least one application supports."""
    apps_root = SDK_ROOT / "application"
    chips = set()
    if apps_root.is_dir():
        for app_dir in apps_root.iterdir():
            vendor = app_dir / "vendor"
            if not vendor.is_dir():
                continue
            for chip_dir in vendor.iterdir():
                if (chip_dir / "app_config" / "Kconfig").is_file():
                    chips.add(chip_dir.name)
    return sorted(chips)


def discover_apps(chip):
    """List applications that have vendor/<chip>/app_config/Kconfig."""
    apps = []
    apps_root = SDK_ROOT / "application"
    if apps_root.is_dir():
        for app_dir in sorted(apps_root.iterdir()):
            if (app_dir / "vendor" / chip / "app_config" / "Kconfig").is_file():
                apps.append(app_dir.name)
    return apps


def prompt_choice(title, items):
    """Numbered terminal picker; accepts a number or the exact name."""
    if not items:
        raise SystemExit(f"error: no choices for {title}")
    if len(items) == 1:
        print(f"{title}: {items[0]}")
        return items[0]

    print(f"\n{title}:")
    for i, item in enumerate(items, 1):
        print(f"  {i}) {item}")
    while True:
        try:
            raw = input(f"Select [1-{len(items)}]: ").strip()
        except EOFError:
            raise SystemExit("\nerror: no input for interactive selection")
        if raw.isdigit() and 1 <= int(raw) <= len(items):
            return items[int(raw) - 1]
        if raw in items:
            return raw
        print("  invalid selection, try again")


def pick_open_sdk_target(chip=None, app=None):
    """Fill in chip/app from the filesystem when omitted on the command line."""
    if chip is None:
        chips = discover_chips()
        if not chips:
            raise SystemExit(
                "error: no chip found under application/*/vendor/*/app_config"
            )
        chip = prompt_choice("Select chip", chips)
    if app is None:
        apps = discover_apps(chip)
        if not apps:
            raise SystemExit(f"error: no application supports chip '{chip}'")
        app = prompt_choice(f"Select application for {chip}", apps)
    return chip, app


VIEW_MARKER = ".scbb_open_sdk_view"


def _link_view_entry(link, target):
    """Create one relative symlink inside the open-SDK source view."""
    link.symlink_to(
        os.path.relpath(target, link.parent),
        target_is_directory=target.is_dir(),
    )


def build_open_sdk_view(view):
    """Populate the filtered view that src_memcpy will flatten.

    Only BSP/axk and the module directories are exposed.  BSP/Ai-M6x and
    BSP/stm32f10x stay in the repo but are unreachable from the build, so
    src_memcpy never copies or compiles them.
    """
    bsp_axk = ROOT / "BSP" / "axk"
    if not bsp_axk.is_dir():
        raise SystemExit(f"error: {bsp_axk} not found")

    view.mkdir()
    (view / VIEW_MARKER).write_text(
        "Generated by adt/adt_scbb/menuconfig.py --open-sdk; safe to delete.\n",
        encoding="utf-8",
    )

    entries = ["BSP/axk"]
    (view / "BSP").mkdir()
    _link_view_entry(view / "BSP" / "axk", bsp_axk)

    skip = {"BSP", "docs", "scripts", "__pycache__"}
    for child in sorted(ROOT.iterdir()):
        if not child.is_dir() or child.name in skip or child.name.startswith("."):
            continue
        _link_view_entry(view / child.name, child)
        entries.append(child.name)

    for header in sorted(ROOT.glob("*.h")):
        _link_view_entry(view / header.name, header)
        entries.append(header.name)

    return entries


def attach_open_sdk_app(chip, app):
    """Make application/<app> build SCBB without touching the open SDK.

    Idempotent:
      * src/adt_scbb is a generated view with symlinks to BSP/axk, the module
        directories and scbb_*.h, so the SDK's src_memcpy flattens only those
        sources (BSP/Ai-M6x and BSP/stm32f10x are never copied or compiled)
      * app_config/Kconfig sources adt/adt_scbb/Kconfig.scbb
    """
    if "/" in app or "\\" in app:
        raise SystemExit(
            f"error: APP must be the directory name under application/ (got '{app}')"
        )

    build_sh = SDK_ROOT / "build.sh"
    if not build_sh.is_file():
        raise SystemExit(
            f"error: {build_sh} not found — this menuconfig.py is not inside "
            "an aithinker_dev_open_sdk checkout"
        )

    app_dir = SDK_ROOT / "application" / app
    if not app_dir.is_dir():
        raise SystemExit(f"error: {app_dir} not found")

    kconfig_path = app_dir / "vendor" / chip / "app_config" / "Kconfig"
    if not kconfig_path.is_file():
        raise SystemExit(
            f"error: {kconfig_path} not found — does {app} support chip {chip}?"
        )

    # 1. filtered source view under the app's src/ so the SDK's src_memcpy
    #    only sees BSP/axk + module sources (never the other BSP platforms)
    src_dir = app_dir / "src"
    src_dir.mkdir(parents=True, exist_ok=True)
    view = src_dir / "adt_scbb"

    if view.is_symlink():
        # Old format: one link to the whole SCBB checkout. Replace it.
        if view.resolve() != ROOT:
            raise SystemExit(
                f"error: {view} already points to {view.resolve()}, not {ROOT}; "
                "remove it manually if you want this script to recreate it"
            )
        view.unlink()
        view_state = "replaced old symlink"
    elif view.exists():
        if not (view / VIEW_MARKER).is_file():
            raise SystemExit(
                f"error: {view} exists but is not a SCBB open-SDK view; "
                "remove it manually or keep your copied component"
            )
        shutil.rmtree(view)
        view_state = "rebuilt"
    else:
        view_state = "created"

    try:
        entries = build_open_sdk_view(view)
    except OSError as e:
        raise SystemExit(
            f"error: cannot create the source view {view}: {e}\n"
            "  On Windows, enable Developer Mode or create it manually."
        )

    # 2. Kconfig source line (resolved from tmp/kconfig/application_kconfig/)
    text = kconfig_path.read_text(encoding="utf-8")
    missing = []
    if "config SCBB_BSP_PLATFORM" not in text:
        missing.append(
            "# AiPi-SCBB: the application (not the SDK) is the board definition.\n"
            "config SCBB_BSP_PLATFORM\n"
            "    string\n"
            '    default "axk"\n'
        )
    if "Kconfig.scbb" not in text:
        missing.append('source "../../../adt/adt_scbb/Kconfig.scbb"\n')
    if missing:
        if not text.endswith("\n"):
            text += "\n"
        kconfig_path.write_text(text + "\n" + "\n".join(missing), encoding="utf-8")
        kconfig_state = "updated"
    else:
        kconfig_state = "exists"

    print(f"[attach] {view} ({view_state}: {', '.join(entries)})")
    print(f"[attach] {kconfig_path} ({kconfig_state})")
    return app_dir


def open_sdk_menuconfig(chip, app, attach_only=False):
    attach_open_sdk_app(chip, app)
    if attach_only:
        print("[open-sdk] attach-only; menuconfig skipped")
        return 0

    cmd = [str(SDK_ROOT / "build.sh"), chip, app, "menuconfig"]
    print(f"[open-sdk] {' '.join(cmd)}", flush=True)
    rc = subprocess.call(cmd, cwd=str(SDK_ROOT))
    if rc == 0:
        staged = SDK_ROOT / "tmp" / "kconfig_h" / chip / app / "aiio_autoconf.h"
        if not staged.is_file():
            print(
                f"warning: {staged} was not generated; run menuconfig again and "
                "make sure it exits cleanly (this header is required by cn debug)",
                file=sys.stderr,
            )
        print(f"\nNext:\n  cd {SDK_ROOT}\n  ./build.sh {chip} {app} cn debug")
    return rc


def standalone_menuconfig():
    cmd = find_menuconfig()
    env = os.environ.copy()
    env["KCONFIG_CONFIG"] = str(ROOT / CONFIG_FILE)
    env["MENUCONFIG_STYLE"] = "aquatic"

    try:
        subprocess.check_call(cmd, cwd=str(ROOT), env=env)
    except subprocess.CalledProcessError as e:
        print(f"\nmenuconfig failed (exit {e.returncode})", file=sys.stderr)
        input("Press Enter to exit...")
        sys.exit(e.returncode)
    except FileNotFoundError:
        print("Error: kconfiglib not installed.\n  pip install kconfiglib windows-curses", file=sys.stderr)
        input("Press Enter to exit...")
        sys.exit(1)

    if not (ROOT / CONFIG_FILE).exists():
        print("Error: .config not generated. Did you save the configuration (press S then Q)?", file=sys.stderr)
        input("Press Enter to exit...")
        sys.exit(1)

    # Read output path from .config
    output_path = CONFIG_HEADER_NAME
    config_path = ROOT / CONFIG_FILE
    with open(config_path, "r") as f:
        for line in f:
            line = line.strip()
            if line.startswith("CONFIG_SCBB_CONFIG_HEADER_PATH="):
                val = line.split("=", 1)[1]
                if len(val) >= 2 and val[0] == '"' and val[-1] == '"':
                    val = val[1:-1]
                output_path = val
                break

    # Ensure uniqueness: remove old scbb_config.h files
    removed = find_and_remove_old_config_headers()
    if removed:
        print(f"Removed old config headers: {', '.join(removed)}")

    # Ensure output directory exists
    output_full = ROOT / output_path
    output_full.parent.mkdir(parents=True, exist_ok=True)

    try:
        subprocess.check_call(
            [sys.executable, "scripts/gen_config_header.py", CONFIG_FILE, output_path],
            cwd=str(ROOT),
        )
        print(f"{output_path} generated successfully.")
    except subprocess.CalledProcessError as e:
        print(f"\nHeader generation failed (exit {e.returncode})", file=sys.stderr)
        input("Press Enter to exit...")
        sys.exit(e.returncode)


def main():
    parser = argparse.ArgumentParser(
        description="AiPi-SCBB configuration menu",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "examples:\n"
            "  python3 menuconfig.py                              # standalone (scbb_config.h)\n"
            "  python3 menuconfig.py --open-sdk                   # pick chip/app, then SDK menuconfig\n"
            "  python3 menuconfig.py --open-sdk bl616cl-32 my_app # open SDK app + SDK menuconfig\n"
            "  python3 menuconfig.py --open-sdk bl616cl-32 my_app --attach-only\n"
        ),
    )
    parser.add_argument(
        "--open-sdk",
        nargs="*",
        metavar="CHIP APP",
        help="attach SCBB to application/<APP> and run ./build.sh <CHIP> <APP> "
             "menuconfig; omit CHIP/APP to pick them from a list",
    )
    parser.add_argument(
        "--attach-only",
        action="store_true",
        help="with --open-sdk: only create the source view and Kconfig source, skip menuconfig",
    )
    args = parser.parse_args()

    if args.attach_only and args.open_sdk is None:
        parser.error("--attach-only requires --open-sdk")

    if args.open_sdk is not None:
        if len(args.open_sdk) > 2:
            parser.error("--open-sdk takes at most CHIP and APP")
        chip = args.open_sdk[0] if len(args.open_sdk) >= 1 else None
        app = args.open_sdk[1] if len(args.open_sdk) == 2 else None
        chip, app = pick_open_sdk_target(chip, app)
        sys.exit(open_sdk_menuconfig(chip, app, args.attach_only))

    standalone_menuconfig()


if __name__ == "__main__":
    main()
