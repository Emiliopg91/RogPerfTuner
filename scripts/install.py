import datetime
import os
import shutil
import subprocess
import requests

rccdc_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "RCCDeckyCompanion"))
openrgb_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "OpenRGB.AppImage"))
udev_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "assets", "60-openrgb.rules"))

pip_dependencies = [
    "black",
    "dataclasses-json",
    "openrgb-python",
    "psutil",
    "pyinstaller",
    "PyQt6",
    "setproctitle",
    "vdf",
]


def install_pip_deps():
    """Install dependencies from PIP"""
    print("  Installing PIP packages...")
    command = ["pip", "install"]
    command.extend(pip_dependencies)
    subprocess.run(command, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    print("    Dependencies installed")


def get_openrgb():
    """Download latest OpenRGB"""
    print("  Installing OpenRGB...")

    url = "https://api.github.com/repos/Emiliopg91/OpenRGB/releases/latest"
    response = requests.get(url)

    file_mod_date = (
        datetime.datetime.fromtimestamp(os.path.getmtime(openrgb_path)) if os.path.exists(openrgb_path) else None
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

                with open(openrgb_path, "wb") as f:
                    for chunk in response.iter_content(chunk_size=8192):
                        f.write(chunk)
                os.chmod(openrgb_path, 0o755)

                print("    Dependency updated, getting udev file...")

                os.chdir(os.path.dirname(openrgb_path))
                subprocess.run(
                    [openrgb_path, "--appimage-extract"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE
                )
                squashfs_root = os.path.join(os.path.dirname(openrgb_path), "squashfs-root")

                src_udev_path = os.path.join(squashfs_root, "usr", "lib", "udev", "rules.d", "60-openrgb.rules")
                shutil.copy2(src_udev_path, udev_path)

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

    url = "https://api.github.com/repos/Emiliopg91/RCCDeckyCompanion/releases/latest"
    response = requests.get(url)

    file_mod_date = (
        datetime.datetime.fromtimestamp(os.path.getmtime(rccdc_path)) if os.path.exists(rccdc_path) else None
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

                with open((rccdc_path + ".tar.gz"), "wb") as f:
                    for chunk in response.iter_content(chunk_size=8192):
                        f.write(chunk)

                print("    Dependency updated, extracting...")

                os.chdir(os.path.dirname(rccdc_path))
                subprocess.run(
                    ["tar", "-xvzf", (rccdc_path + ".tar.gz"), "-C", os.path.dirname(rccdc_path)],
                    check=True,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.PIPE,
                )
                os.unlink((rccdc_path + ".tar.gz"))
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
