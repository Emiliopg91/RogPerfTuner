# pylint: disable=import-outside-toplevel
import datetime
import os
import shutil
import subprocess

RCCDC_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "RCCDeckyCompanion"))
OPENRGB_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "OpenRGB.AppImage"))
UDEV_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "60-openrgb.rules"))

DEVELOP_DEPENDENCIES = [
    "black",
    "pyinstaller",
    "pylint",
    "requests",
    "watchdog",
]

RUNTIME_DEPENDENCIES = [
    "cryptography",
    "keyring",
    "dataclasses-json",
    "psutil",
    "pyyaml",
    "pyudev",
    "PyQt5",
    "qasync",
    "setproctitle",
    "websockets",
]


def install_pip_deps():
    """Install dependencies from PIP"""
    print("  Installing PIP packages...")
    command = ["python", "-m", "pip", "install"]
    command.extend(DEVELOP_DEPENDENCIES)
    command.extend(RUNTIME_DEPENDENCIES)
    subprocess.run(command, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    print("    Dependencies installed")


def get_openrgb():
    """Download latest OpenRGB"""
    print("  Installing OpenRGB...")
    import requests

    url = "https://api.github.com/repos/Emiliopg91/OpenRGB/releases/latest"
    response = requests.get(url)

    file_mod_date = (
        datetime.datetime.fromtimestamp(os.path.getmtime(OPENRGB_PATH)) if os.path.exists(OPENRGB_PATH) else None
    )

    if response.status_code == 200:
        release_data = response.json()
        data = [asset for asset in release_data["assets"] if asset["name"].endswith(".AppImage")]
        if len(data) > 0:
            asset = data[0]
            if file_mod_date is None or file_mod_date < datetime.datetime.strptime(
                asset["created_at"], "%Y-%m-%dT%H:%M:%SZ"
            ):
                print("    Downloading dependency...")
                url = asset["browser_download_url"]

                response = requests.get(url, stream=True)
                response.raise_for_status()

                with open(OPENRGB_PATH, "wb") as f:
                    for chunk in response.iter_content(chunk_size=8192):
                        f.write(chunk)
                os.chmod(OPENRGB_PATH, 0o755)

                print("    Dependency updated, getting udev file...")

                os.chdir(os.path.dirname(OPENRGB_PATH))
                subprocess.run(
                    [OPENRGB_PATH, "--appimage-extract"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE
                )
                squashfs_root = os.path.join(os.path.dirname(OPENRGB_PATH), "squashfs-root")

                src_udev_path = os.path.join(squashfs_root, "usr", "lib", "udev", "rules.d", "60-openrgb.rules")
                shutil.copy2(src_udev_path, UDEV_PATH)

                shutil.rmtree(squashfs_root)
                print("    Udev file extracted")
            else:
                print("    Dependency up to date")
        else:
            raise FileNotFoundError("No URL published for OpenRGB")
    else:
        raise Exception(f"Error getting latest release: {response.status_code}")


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
                subprocess.run(
                    ["tar", "-xvzf", (RCCDC_PATH + ".tar.gz"), "-C", os.path.dirname(RCCDC_PATH)],
                    check=True,
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
install_pip_deps()
get_openrgb()
get_rccdc()
