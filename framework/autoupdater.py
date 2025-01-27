from threading import Thread

import os
import time
import tempfile
import shutil
from typing import Callable

import requests

from framework.logger import Logger


class Asset:
    """Class for hold asset data"""

    def __init__(self, url, size):
        self._url = url
        self._size = size

    def __repr__(self):
        return f"Asset(url={self._url}, size={self._size})"


class AutoUpdater:
    """Class to manage application updates"""

    def __init__(  # pylint: disable=R0913,R0917
        self,
        app_name: str,
        version: str,
        owner: str,
        repository: str,
        user_update_folder: str,
        restart_method: Callable[[], None],
        dev_mode=False,
    ):
        self._logger = Logger()
        self._app_image = os.getenv("APPIMAGE")

        self.app_name = app_name
        self.version = version
        self.dev_mode = dev_mode
        self.user_update_folder = user_update_folder
        self.restart_method = restart_method

        if self._app_image is not None:
            self._logger.debug(f"AppImage location: {self._app_image}")
            self._owner = owner
            self._repository = repository
            self._update_path = None

            self._logger.debug(f"AutoUpdater configured for repository {self._owner}/{self._repository}")
        else:
            self._logger.warning("Auto update is only available for AppImage version")

    def start(self) -> None:
        """Start auto update checks"""
        if self._app_image is not None:
            Thread(name="AutoUpdater", target=self._check_task).start()

    def _check_task(self) -> None:
        time.sleep(1)
        while self._update_path is None:
            self._logger.info("Checking for updates...")
            data = self.get_update_url()
            if data is None:
                self._logger.info("No update found")
                time.sleep(3600)
            else:
                self.download_update(data._url)  # pylint: disable=W0212
                if not self.dev_mode:
                    self.copy_file(self._update_path)
                    self.restart_method()

    def copy_file(self, tmp_file: str) -> None:
        """Copy temporal file to pending update path"""
        try:
            shutil.move(tmp_file, os.path.join(self.user_update_folder, f"{self.app_name}.AppImage"))
        except Exception as e:
            self._logger.error(f"Error while copying file: {e}")

    def get_update_url(self) -> Asset | None:
        """Retrieve download url for asset"""
        url = f"https://api.github.com/repos/{self._owner}/{self._repository}/releases/latest"
        response = requests.get(url)

        if response.status_code == 200:
            release_data = response.json()
            remote_version = release_data["tag_name"]
            if self.is_newer(remote_version):
                self._logger.info(f"Update found for version {remote_version}")
                data = [
                    Asset(asset["browser_download_url"], asset["size"])
                    for asset in release_data["assets"]
                    if asset["name"].endswith(".AppImage")
                ]
                if len(data) > 0:
                    return data[0]
        else:
            self._logger.error(f"Error getting latest release: {response.status_code}")

        return None

    def download_update(self, url: str) -> None:
        """Download update file from url"""
        with tempfile.NamedTemporaryFile(delete=False) as temp_file:
            temp_file_path = temp_file.name
            self._logger.info("Downloading update")

            try:
                response = requests.get(url, stream=True)
                response.raise_for_status()

                with open(temp_file_path, "wb") as f:
                    for chunk in response.iter_content(chunk_size=8192):
                        f.write(chunk)

                self._logger.info("Download completed")

                self._update_path = temp_file_path

            except requests.exceptions.RequestException as e:
                self._logger.error(f"Error downloading the file: {e}")

    def is_newer(self, version: str):
        """Compare current version with parameter"""

        def parse_version(version):
            return tuple(map(int, version.split(".")))

        v1 = parse_version(self.version)
        v2 = parse_version(version)

        return v1 < v2
