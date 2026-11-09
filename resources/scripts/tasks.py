# pylint: disable=missing-function-docstring
"""Project tasks called from the Makefile: bundled assets, versioning and local install."""

import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SUBMODULE_DIR = ROOT / "submodules"
PATCH_DIR = SUBMODULE_DIR / "patches"
OPENRGB_DIR = SUBMODULE_DIR / "OpenRGB"
RCCDC_DIR = SUBMODULE_DIR / "RccDeckyCompanion"
ASSETS_DIR = ROOT / "build" / "assets"

os.chdir(ROOT)


def run(cmd, cwd=None):
    subprocess.run(cmd, cwd=cwd, check=True, env=os.environ.copy())


def banner(text):
    print("#" * 71)
    print(f" {text} ".center(71, "#"))
    print("#" * 71)


# ---------------- assets ----------------


def patch():
    for patch_file in sorted(PATCH_DIR.glob("*.diff")):
        applied = patch_file.with_suffix(".diff.applied")
        submodule_path = SUBMODULE_DIR / patch_file.stem
        if applied.exists() or not submodule_path.exists():
            continue
        banner(f"Applying patch to {patch_file.stem}")
        run(
            ["git", "apply", "--whitespace=nowarn", f"../patches/{patch_file.name}"],
            cwd=submodule_path,
        )
        applied.touch()


def link_openrgb_json():
    """The patched OpenRGB build expects `nlohmann-json/include`: point it to OpenRGB's own copy."""
    link_dir = OPENRGB_DIR / "nlohmann-json"
    if link_dir.is_symlink():
        # Old layout: link to the removed nlohmann-json submodule
        link_dir.unlink()
    if not (link_dir / "include").exists():
        link_dir.mkdir(exist_ok=True)
        (link_dir / "include").symlink_to("../dependencies/json")


def build_openrgb():
    target = ASSETS_DIR / "OpenRGB"
    if target.exists():
        return
    banner("Compiling OpenRGB")
    link_openrgb_json()
    if not (OPENRGB_DIR / "build").exists():
        (OPENRGB_DIR / "configure.sh").chmod(0o755)
        run(["./configure.sh"], cwd=OPENRGB_DIR)
    run(["./build.sh"], cwd=OPENRGB_DIR)
    target.mkdir(parents=True)
    shutil.copy(OPENRGB_DIR / "build/OpenRGB", target / "openrgb")
    shutil.copy(OPENRGB_DIR / "60-openrgb.rules", target / "60-openrgb.rules")


def build_rccdc():
    target = ASSETS_DIR / "RccDeckyCompanion"
    if target.exists():
        return
    banner("Compiling RccDeckyCompanion")
    run(["./cli/decky.py", "build"], cwd=RCCDC_DIR)
    shutil.copytree(RCCDC_DIR / "out/RccDeckyCompanion", target, dirs_exist_ok=True)


def assets():
    """Everything the app loads from build/assets (/usr/share/rog-perf-tuner once installed)."""
    patch()
    link_openrgb_json()
    build_openrgb()
    build_rccdc()
    shutil.copytree(ROOT / "resources/icons", ASSETS_DIR / "icons", dirs_exist_ok=True)
    shutil.copy(ROOT / "resources/suggestions.yaml", ASSETS_DIR / "suggestions.yaml")
    shutil.copy(ROOT / "resources/dev-mode.sh", ASSETS_DIR / "dev-mode.sh")
    (ASSETS_DIR / "dev-mode.sh").chmod(0o755)


# ---------------- clean ----------------


def clean():
    banner("Cleaning workspace")
    for d in ["build", "dist", "node_modules", "src-tauri/target"]:
        shutil.rmtree(ROOT / d, ignore_errors=True)
    for p in PATCH_DIR.glob("*.applied"):
        p.unlink()
    # Only inside initialized submodules: otherwise git would act on the main repository
    if (OPENRGB_DIR / ".git").exists():
        run(["git", "reset", "--hard"], cwd=OPENRGB_DIR)
        run(["git", "clean", "-fdx"], cwd=OPENRGB_DIR)
    if (RCCDC_DIR / ".git").exists():
        run(["git", "reset", "--hard"], cwd=RCCDC_DIR)


# ---------------- versioning ----------------


def get_version():
    content = (ROOT / "src-tauri/Cargo.toml").read_text(encoding="utf-8")
    return re.search(r'^version\s*=\s*"([^"]+)"', content, re.M).group(1)


def increase_version():
    major, minor, patch_v = map(int, get_version().split("."))
    new = f"{major}.{minor}.{patch_v + 1}"

    cargo = ROOT / "src-tauri/Cargo.toml"
    content = cargo.read_text(encoding="utf-8")
    content = re.sub(r'^version\s*=\s*"[^"]+"', f'version = "{new}"', content, count=1, flags=re.M)
    cargo.write_text(content, encoding="utf-8")
    for file in [ROOT / "package.json", ROOT / "src-tauri/tauri.conf.json"]:
        data = json.loads(file.read_text(encoding="utf-8"))
        data["version"] = new
        file.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    print(f"Version {new}")


# ---------------- install ----------------


def install():
    """Builds and installs a git package from the local working copy."""
    clean()
    os.environ["GIT_RELEASE"] = "1"
    run(["make", "release"])

    pkgbuild = ROOT / "dist/PKGBUILD"
    with pkgbuild.open("a", encoding="utf-8") as f:
        f.write("\nprepare() {\n")
        f.write('    rm -Rf "$srcdir/RogPerfTuner/*"\n')
        excludes = "--exclude='dist' --exclude='node_modules' --exclude='target'"
        f.write(f'    rsync -a {excludes} {ROOT}/ "$srcdir/RogPerfTuner/"\n')
        f.write("}\n")

    run(["makepkg", "-si"], cwd=ROOT / "dist")


COMMANDS = {
    "assets": assets,
    "clean": clean,
    "increase_version": increase_version,
    "install": install,
    "version": lambda: print(get_version()),
}

if __name__ == "__main__":
    for name in sys.argv[1:] or ["assets"]:
        if name not in COMMANDS:
            print(f"Unknown command: {name}. Available: {' '.join(COMMANDS)}", file=sys.stderr)
            sys.exit(1)
        try:
            COMMANDS[name]()
        except subprocess.CalledProcessError as e:
            print(e, file=sys.stderr)
            sys.exit(e.returncode)
