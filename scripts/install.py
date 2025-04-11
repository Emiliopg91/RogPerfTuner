# pylint: disable=import-outside-toplevel
import datetime
import os
import shutil
import subprocess
import sys
import time

RCCDC_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "RCCDeckyCompanion"))
OPENRGB_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "OpenRGB"))
UDEV_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "60-openrgb.rules"))


def install_uv_deps():
    """Install uv dependencies"""
    print("  Installing project dependencies...")
    command = " ".join(["uv", "sync", "--reinstall"])
    subprocess.run(command, shell=True, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)


def get_last_mod_time(directorio):
    ultima_fecha = None

    for carpeta_raiz, _, archivos in os.walk(directorio):
        for archivo in archivos:
            ruta_completa = os.path.join(carpeta_raiz, archivo)
            try:
                timestamp = os.path.getmtime(ruta_completa)
                if ultima_fecha is None or timestamp > ultima_fecha:
                    ultima_fecha = timestamp
            except Exception:
                pass
    return ultima_fecha


def get_openrgb():
    """Download latest OpenRGB"""
    print("  Building OpenRGB...")

    if not os.path.exists(OPENRGB_PATH) or get_last_mod_time(OPENRGB_PATH) < get_last_mod_time(
        os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "submodules", "OpenRGB"))
    ):
        if os.path.exists(OPENRGB_PATH):
            shutil.rmtree(OPENRGB_PATH)
        subprocess.run(
            "./build.sh",
            cwd=os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "submodules", "OpenRGB")),
            check=True,
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        shutil.copy2("./submodules/OpenRGB/OpenRGB.AppImage", OPENRGB_PATH + ".AppImage")
        os.chmod(OPENRGB_PATH + ".AppImage", 0o755)

        os.chdir(os.path.dirname(OPENRGB_PATH))
        command = " ".join([OPENRGB_PATH + ".AppImage", "--appimage-extract"])
        subprocess.run(
            command,
            shell=True,
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        squashfs_root = os.path.join(os.path.dirname(OPENRGB_PATH), "squashfs-root")
        shutil.move(squashfs_root, OPENRGB_PATH)
        os.unlink(OPENRGB_PATH + ".AppImage")
        os.unlink(os.path.join(OPENRGB_PATH, "usr", "bin", "OpenRGB.exe"))
        subprocess.run(f"chmod 777 -R {OPENRGB_PATH}", shell=True, check=True)


def get_rccdc():
    """Download latest RCCDeckyCompanion"""
    print("  Building RCCDeckyCompanion...")

    if not os.path.exists(RCCDC_PATH) or os.path.getmtime(RCCDC_PATH) < get_last_mod_time(
        os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "submodules", "RCCDeckyCompanion"))
    ):
        if os.path.exists(RCCDC_PATH):
            shutil.rmtree(RCCDC_PATH)
        subprocess.run(
            "./cli/decky.py build",
            cwd=os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "submodules", "RCCDeckyCompanion")),
            check=True,
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )

        shutil.copytree(
            os.path.abspath(
                os.path.join(
                    os.path.dirname(__file__), "..", "submodules", "RCCDeckyCompanion", "out", "RCCDeckyCompanion"
                )
            ),
            RCCDC_PATH,
        )
        subprocess.run(f"touch {RCCDC_PATH}", shell=True, check=True)


print("Installing dependencies...")
install_uv_deps()
if not "skipSubmodules" in sys.argv:
    get_openrgb()
    get_rccdc()
