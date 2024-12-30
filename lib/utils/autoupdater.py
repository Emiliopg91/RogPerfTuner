from .. import is_newer, __app_name__
from .application import application
from .constants import autoupdate_path, user_update_folder
from .logger import Logger
from threading import Thread

import json
import os
import requests
import shutil
import tempfile
import time


class Asset:
    def __init__(self, url, size):
        self.url = url
        self.size = size

    def __repr__(self):
        return f"Asset(url={self.url}, size={self.size})"


class AutoUpdater:
    def __init__(self):
        self.logger = Logger(self.__class__.__name__)
        self.app_image = os.getenv("APPIMAGE")

        if self.app_image is not None:
            self.logger.info(f"AppImage location: {self.app_image}")
            with open(autoupdate_path, "r") as file:
                data = json.load(file)
            self.owner = data["owner"]
            self.repository = data["repository"]
            self.update_path = None

            self.logger.info(
                f"AutoUpdater configured for repository {self.owner}/{self.repository}"
            )
        else:
            self.logger.warning("Auto update is only available for AppImage version")

    def start(self):
        if self.app_image is not None:
            Thread(name="AutoUpdater", target=self._check_task).start()

    def _check_task(self):
        time.sleep(1)
        while self.update_path is None:
            self.logger.info("Checking for updates...")
            data = self.get_update_url()
            if data is None:
                self.logger.info("No update found")
                time.sleep(3600)
            else:
                self.download_update(data.url, data.size)
                self.copy_file(self.update_path)
                application.relaunch_application()

    def copy_file(self, tmp_file):
        try:
            shutil.copy2(
                tmp_file, os.path.join(user_update_folder, f"{__app_name__}.AppImage")
            )
        except Exception as e:
            self.logger.error(f"Error while copying file: {e}")

    def get_update_url(self) -> Asset | None:
        url = f"https://api.github.com/repos/{self.owner}/{self.repository}/releases/latest"
        response = requests.get(url)

        if response.status_code == 200:
            release_data = response.json()
            remote_version = release_data["tag_name"]
            if is_newer(remote_version):
                self.logger.info(f"Update found for version {remote_version}")
                data = [
                    Asset(asset["browser_download_url"], asset["size"])
                    for asset in release_data["assets"]
                    if asset["name"].endswith(".AppImage")
                ]
                if len(data) > 0:
                    return data[0]
        else:
            print(f"Error getting latest release: {response.status_code}")

        return None

    def download_update(self, url, size):
        with tempfile.NamedTemporaryFile(delete=False) as temp_file:
            temp_file_path = temp_file.name
            self.logger.info(f"Downloading {size} bytes from {url} to {temp_file_path}")

            try:
                start_time = time.time()
                response = requests.get(url, stream=True)
                response.raise_for_status()
                total_downloaded = 0

                with open(temp_file_path, "wb") as f:
                    for chunk in response.iter_content(chunk_size=8192):
                        f.write(chunk)
                        total_downloaded += len(chunk)

                end_time = time.time()
                download_time = end_time - start_time
                speed = total_downloaded / download_time

                speed_mb_per_s = speed / (1024 * 1024)
                self.logger.info(
                    f"Download completed in {download_time:.2f} seconds ({speed_mb_per_s:.2f} MB/s)"
                )

                self.update_path = temp_file_path

            except requests.exceptions.RequestException as e:
                self.logger.error(f"Error downloading the file: {e}")


auto_updater = AutoUpdater()
