# pylint: disable=import-outside-toplevel
import datetime
import os
import shutil
import subprocess
import time

RCCDC_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "RCCDeckyCompanion"))
OPENRGB_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "OpenRGB.AppImage"))
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

    if not os.path.exists(OPENRGB_PATH) or os.path.getmtime(OPENRGB_PATH) < get_last_mod_time(
        os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "submodules", "OpenRGB"))
    ):
        if os.path.exists(OPENRGB_PATH):
            os.unlink(OPENRGB_PATH)
        subprocess.run(
            "./build.sh",
            cwd=os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "submodules", "OpenRGB")),
            check=True,
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        shutil.copy2("./submodules/OpenRGB/OpenRGB-Exp.AppImage", OPENRGB_PATH)
        os.chmod(OPENRGB_PATH, 0o755)

        print("    Built, getting udev file...")

        os.chdir(os.path.dirname(OPENRGB_PATH))
        command = " ".join([OPENRGB_PATH, "--appimage-extract"])
        subprocess.run(command, shell=True, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        squashfs_root = os.path.join(os.path.dirname(OPENRGB_PATH), "squashfs-root")

        src_udev_path = os.path.join(squashfs_root, "usr", "lib", "udev", "rules.d", "60-openrgb.rules")
        shutil.copy2(src_udev_path, UDEV_PATH)

        shutil.rmtree(squashfs_root)
        print("    Udev file extracted")


def get_rccdc():
    """Download latest RCCDeckyCompanion"""
    print("  Installing RCCDeckyCompanion...")
    import requests

    url = "https://api.github.com/repos/Emiliopg91/RCCDeckyCompanion/releases/latest"
    response = requests.get(url)

    file_mod_date = (
        datetime.datetime.fromtimestamp(os.path.getmtime(RCCDC_PATH)) if os.path.exists(RCCDC_PATH) else None
    )

    if response.status_code == 200:
        release_data = response.json()
        data = [asset for asset in release_data["assets"] if asset["name"].endswith(".tar.gz")]
        if len(data) > 0:
            asset = data[0]
            if file_mod_date is None or file_mod_date < datetime.datetime.strptime(
                asset["created_at"], "%Y-%m-%dT%H:%M:%SZ"
            ):
                print("    Downloading dependency...")
                url = asset["browser_download_url"]

                response = requests.get(url, stream=True)
                response.raise_for_status()

                with open((RCCDC_PATH + ".tar.gz"), "wb") as f:
                    for chunk in response.iter_content(chunk_size=8192):
                        f.write(chunk)

                print("    Dependency updated, extracting...")

                os.chdir(os.path.dirname(RCCDC_PATH))
                command = " ".join(["tar", "-xvzf", (RCCDC_PATH + ".tar.gz"), "-C", os.path.dirname(RCCDC_PATH)])
                subprocess.run(
                    command,
                    check=True,
                    shell=True,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.PIPE,
                )
                os.unlink((RCCDC_PATH + ".tar.gz"))
                print("    File extracted")
            else:
                print("    Dependency up to date")
        else:
            raise FileNotFoundError("No URL published for OpenRGB")
    else:
        raise Exception(f"Error getting latest release: {response.status_code}")


print("Installing dependencies...")
time.sleep(0.1)
install_uv_deps()
get_openrgb()
get_rccdc()
