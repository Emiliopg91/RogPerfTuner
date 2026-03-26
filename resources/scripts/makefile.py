import os
import sys
import shutil
import subprocess
from pathlib import Path
import pty
import re


NUM_CORES = os.cpu_count()
BUILD_TYPE = os.environ.get("BUILD_TYPE", "Release")
ROOT = Path(os.path.join(os.path.dirname(__file__), "..", "..")).resolve()
SUBMODULE_DIR = ROOT / "submodules"
PATCH_DIR = SUBMODULE_DIR / "patches"
FROZEN_SUBMODULES = [SUBMODULE_DIR / "OpenRGB-cppSDK", SUBMODULE_DIR / "OpenRGB"]

os.chdir(ROOT)


# ---------------- Helpers ----------------
def run(cmd, cwd=None):
    subprocess.run(cmd, cwd=cwd, check=True, env=os.environ.copy())


def safe_unlink(path: Path):
    try:
        path.unlink()
    except FileNotFoundError:
        pass


def ensure_dir(path: Path):
    path.mkdir(parents=True, exist_ok=True)


# ---------------- clean ----------------


def clean():
    print("#######################################################################")
    print("######################### Cleaning workspace ##########################")
    print("#######################################################################")

    for d in ["build", "dist", ".Debug", ".Release", ".qt", "logs", "out"]:
        if (ROOT / d).exists():
            if (ROOT / d).is_dir():
                shutil.rmtree(ROOT / d)
            else:
                (ROOT / d).unlink()

    for p in ROOT.rglob("cmake_install.cmake"):
        safe_unlink(p)

    for p in ROOT.rglob("CMakeFiles"):
        shutil.rmtree(p, ignore_errors=True)

    for p in ROOT.rglob("config.cmake"):
        safe_unlink(p)

    for p in PATCH_DIR.glob("*.diff.applied"):
        safe_unlink(p)

    # Submodules
    run(["git", "reset", "--hard"], cwd=SUBMODULE_DIR / "OpenRGB-cppSDK")
    run(
        ["git", "submodule", "foreach", "git", "reset", "--hard"],
        cwd=SUBMODULE_DIR / "OpenRGB-cppSDK",
    )

    run(["git", "reset", "--hard"], cwd=SUBMODULE_DIR / "OpenRGB")
    run(["git", "clean", "-fdx"], cwd=SUBMODULE_DIR / "OpenRGB")

    safe_unlink(SUBMODULE_DIR / "OpenRGB" / "CMakeLists.txt")

    run(["git", "reset", "--hard"], cwd=SUBMODULE_DIR / "RccDeckyCompanion")


# ---------------- patch ----------------


def patch():
    print("#######################################################################")
    print("########################## Applying patches ###########################")
    print("#######################################################################")

    for patch_file in PATCH_DIR.iterdir():
        if patch_file.suffix not in (".diff", ".patch"):
            continue

        applied = patch_file.with_suffix(patch_file.suffix + ".applied")
        if applied.exists():
            continue

        name = patch_file.name
        submodule = patch_file.stem
        submodule_path = SUBMODULE_DIR / submodule

        if not submodule_path.exists():
            print(f"[WARN] Submodule {submodule} does not exist, skipping patch")
            continue

        print(f"Applying patch to {submodule}")
        run(
            ["git", "apply", "--whitespace=nowarn", f"../patches/{name}"],
            cwd=submodule_path,
        )
        applied.touch()


# ---------------- config ----------------


def config():
    # limpiar parcialmente
    for d in ["build", "dist", ".Debug", ".Release"]:
        shutil.rmtree(ROOT / d, ignore_errors=True)

    for p in ROOT.rglob("cmake_install.cmake"):
        safe_unlink(p)

    for p in ROOT.rglob("CMakeFiles"):
        shutil.rmtree(p, ignore_errors=True)

    patch()

    print("#######################################################################")
    print("######################## Configuring compiler #########################")
    print("#######################################################################")

    os.environ["CMAKE_COLOR"] = "1"
    run(
        [
            "cmake",
            "-B",
            "build",
            "-G",
            "Ninja",
            "-DCMAKE_CXX_COMPILER=clang++",
            "-S",
            ".",
            f"-DCMAKE_BUILD_TYPE={BUILD_TYPE}",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        ]
    )

    openrgb = SUBMODULE_DIR / "OpenRGB"
    (openrgb / "configure.sh").chmod(0o755)
    run(["./configure.sh"], cwd=openrgb)

    cc_file = ROOT / "compile_commands.json"
    if not cc_file.exists():
        cc_file.symlink_to("build/compile_commands.json")

    ensure_dir(ROOT / "build/assets")
    shutil.copytree(
        ROOT / "resources/icons", ROOT / "build/assets/icons", dirs_exist_ok=True
    )

    Path(f".{BUILD_TYPE}").touch()


# ---------------- format ----------------


def format_code():
    files = (
        list(Path("RogPerfTuner").rglob("*.cpp"))
        + list(Path("RogPerfTuner").rglob("*.hpp"))
        + list(Path("Framework").rglob("*.cpp"))
        + list(Path("Framework").rglob("*.hpp"))
    )

    if files:
        run(["clang-format", "-i", *map(str, files)])


# ---------------- build ----------------


def build():
    if not Path(f".{BUILD_TYPE}").exists():
        config()

    build_openrgb()
    build_rccdc()

    print("#######################################################################")
    print("####################### Compiling RogPerfTuner ########################")
    print("#######################################################################")

    shutil.rmtree("build/assets/bin", ignore_errors=True)

    format_code()

    run(["cmake", "--build", "build", "--", f"-j{NUM_CORES}"])

    base = Path("build/assets/bin")
    for p in ["rgb", "performance", "steam"]:
        ensure_dir(base / p)

    shutil.copy("resources/suggestions.yaml", "build/assets/suggestions.yaml")
    shutil.copy("resources/translations.yaml", "build/assets/translations.yaml")
    shutil.copy("resources/dev-mode.sh", "build/assets/dev-mode.sh")
    Path("build/assets/dev-mode.sh").chmod(0o755)


def pkgbuild():
    # forzar BUILD_TYPE Release
    global BUILD_TYPE
    BUILD_TYPE = "Release"
    os.environ["BUILD_TYPE"] = BUILD_TYPE

    # construir todo en Release
    build()

    # crear dist
    dist_dir = ROOT / "dist"
    ensure_dir(dist_dir)

    # copiar completion
    shutil.copy(ROOT / "resources/completion-bash", dist_dir / "completion-bash")


# ---------------- build_openrgb ----------------


def build_openrgb():
    target = Path("build/assets/OpenRGB")
    if not target.exists():
        print("#######################################################################")
        print("######################### Compiling OpenRGB ###########################")
        print("#######################################################################")

        run(["./build.sh"], cwd=SUBMODULE_DIR / "OpenRGB")
        ensure_dir(target)
        shutil.copy(SUBMODULE_DIR / "OpenRGB/build/OpenRGB", target / "openrgb")
        shutil.copy(
            SUBMODULE_DIR / "OpenRGB/60-openrgb.rules", target / "60-openrgb.rules"
        )


# ---------------- build_rccdc ----------------


def build_rccdc():
    if Path(".Debug").exists():
        target = Path("build/assets/RccDeckyCompanion")
        if not target.exists():
            print(
                "#######################################################################"
            )
            print(
                "#################### Compiling RccDeckyCompanion ######################"
            )
            print(
                "#######################################################################"
            )

            run(["./cli/decky.py", "build"], cwd=SUBMODULE_DIR / "RccDeckyCompanion")
            ensure_dir(target)
            shutil.copytree(
                SUBMODULE_DIR / "RccDeckyCompanion/out/RccDeckyCompanion",
                target,
                dirs_exist_ok=True,
            )


# ---------------- run ----------------


def run_app():
    build_debug()
    app = ROOT / "build/RogPerfTuner/RogPerfTuner"
    os.utime(app, (1445472000, 1445472000))
    env = os.environ.copy()
    env["RCC_ASSETS_DIR"] = str(ROOT / "build/assets")
    print(f"Running RCC_ASSETS_DIR={env['RCC_ASSETS_DIR']}")
    subprocess.run([str(app)], env=env, check=True)


# ---------------- increase_version ----------------


def increase_version():
    pattern = re.compile(r"([0-9]+)\.([0-9]+)\.([0-9]+)")
    tmp_file = ROOT / "CMakeLists.txt.tmp"
    with open(ROOT / "CMakeLists.txt") as f, open(tmp_file, "w") as out:
        for line in f:
            if "project(" in line and "VERSION" in line:
                m = pattern.search(line)
                if m:
                    major, minor, patch = map(int, m.groups())
                    patch += 1
                    line = pattern.sub(f"{major}.{minor}.{patch}", line)
            out.write(line)
    tmp_file.replace(ROOT / "CMakeLists.txt")


# ---------------- test ----------------


def test():
    os.environ["GIT_RELEASE"] = "1"
    os.environ["IN_TEST"] = "1"
    release()
    run(["python3", "./resources/scripts/test.py"])


# ---------------- release ----------------


def release():
    shutil.rmtree(ROOT / "dist", ignore_errors=True)
    ensure_dir(ROOT / "dist")
    os.chmod(ROOT / "dist", 0o777)

    shutil.copy(ROOT / "resources/PKGBUILD", ROOT / "dist/PKGBUILD")
    if os.environ.get("GIT_RELEASE"):
        shutil.copy(
            ROOT / "resources/rog-perf-tuner.sh",
            ROOT / "dist/rog-perf-tuner-git.install",
        )
    else:
        shutil.copy(
            ROOT / "resources/rog-perf-tuner.sh", ROOT / "dist/rog-perf-tuner.install"
        )

    run(["python", "resources/scripts/release.py"])


# ---------------- build_debug ----------------


def build_debug():
    os.environ["DEV_MODE"] = "1"
    global BUILD_TYPE
    BUILD_TYPE = "Debug"
    build()


# ---------------- install ----------------


def install():
    clean()
    os.environ["GIT_RELEASE"] = "1"
    release()

    pkgbuild = ROOT / "dist/PKGBUILD"
    with pkgbuild.open("a") as f:
        f.write("\nprepare() {\n")
        f.write('    rm -Rf "$srcdir/RogPerfTuner/*"\n')
        f.write(f"    rsync -a --exclude='dist' {ROOT} \"$srcdir/RogPerfTuner/\"\n")
        f.write("    cur=$(pwd)\n")
        f.write('    cd "$srcdir/RogPerfTuner/"\n')
        f.write(
            '    rm -Rf "$srcdir/RogPerfTuner/submodules/RccDeckyCompanion/node_modules"\n'
        )
        f.write("    make clean\n")
        f.write("    cd $cur\n")
        f.write("}\n")

    run(["makepkg", "-si"], cwd=ROOT / "dist")


# ---------------- update_submodules ----------------


def update_submodules():
    clean()
    print("#######################################################################")
    print("######################## Updating submodules ##########################")
    print("#######################################################################")

    out = subprocess.check_output(
        ["git", "config", "--file", ".gitmodules", "--get-regexp", "path"], text=True
    )
    paths = [Path(line.split()[1]) for line in out.splitlines()]

    for path in paths:
        if (ROOT / path) in FROZEN_SUBMODULES:
            continue
        print(f"Updating {path}")
        branch = subprocess.check_output(
            [
                "git",
                "config",
                "--file",
                ".gitmodules",
                "--get",
                f"submodule.{path}.branch",
            ],
            text=True,
        ).strip()
        run(["git", "checkout", branch], cwd=path)
        run(["git", "pull"], cwd=path)


# ---------------- CLI ----------------


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "run"

    commands = {
        "clean": clean,
        "patch": patch,
        "config": config,
        "build": build,
        "build_openrgb": build_openrgb,
        "build_rccdc": build_rccdc,
        "format": format_code,
        "pkgbuild": pkgbuild,
        "release": release,
        "build_debug": build_debug,
        "run": run_app,
        "increase_version": increase_version,
        "test": test,
        "install": install,
        "update_submodules": update_submodules,
    }

    if cmd in commands:
        try:
            commands[cmd]()
        except KeyboardInterrupt:
            sys.exit(130)
        except subprocess.CalledProcessError as e:
            print(f"{e}")
            sys.exit(e.returncode)
    else:
        print(f"Unknown command: {cmd}")
        sys.exit(1)


if __name__ == "__main__":
    main()
