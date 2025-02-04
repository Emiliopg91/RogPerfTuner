import os
import shutil
import signal
import subprocess

from rcc import __app_name__
from rcc.gui.notifier import NOTIFIER
from rcc.utils.beans import translator
from rcc.utils.constants import (
    APP_DRAW_FILE,
    AUTOSTART_FILE,
    ICONS_PATH,
    USER_BIN_FOLDER,
    USER_ICON_FOLDER,
    USER_UPDATE_FOLDER,
)
from rcc.utils.beans import event_bus
from framework.logger import Logger


class Application:
    """Class for managing applicaion aspects"""

    def __init__(self):
        self._logger = Logger()
        self._runner_file = os.path.join(USER_BIN_FOLDER, "launch.sh")
        self._rccdc_enabled = False

        self._desktop_content = f"""[Desktop Entry]
Exec={self._runner_file}
Icon={os.path.join(USER_ICON_FOLDER, "icon.svg")}
Name={__app_name__}
Comment=An utility to manage Asus Rog laptop performance
Path=
Terminal=False
Type=Application
Categories=Utility;
    """
        shutil.copy2(
            os.path.join(ICONS_PATH, "rog-logo.svg"),
            os.path.join(USER_ICON_FOLDER, "icon.svg"),
        )

    def generate_run(self) -> None:
        """Generate runner file"""
        update_file = f"{os.path.join(USER_UPDATE_FOLDER, __app_name__)}.AppImage"
        content = f"""#!/bin/bash

if [[ -f "{update_file}" ]]; then
    cp "{update_file}" "{os.getenv("APPIMAGE")}"
    chmod 755 "{os.getenv("APPIMAGE")}"
fi

"{os.getenv("APPIMAGE")}"
"""
        with open(self._runner_file, "w", encoding="utf-8") as file:
            file.write(content)
        os.chmod(self._runner_file, 0o755)

        self._logger.debug(f"Launch file '{self._runner_file}' written successfully")

    def enable_autostart(self) -> None:
        """Create file to enable autostart on login"""
        self._logger.debug("Creating autostart file")
        dir_path = os.path.dirname(AUTOSTART_FILE)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path, exist_ok=True)

        with open(AUTOSTART_FILE, "w", encoding="utf-8") as file:
            file.write(self._desktop_content)

        self._logger.debug(f"Autostart file '{AUTOSTART_FILE}' written successfully")

    def create_menu_entry(self) -> None:
        """Create file to add application to menu"""
        self._logger.debug("Creating app menu file")
        dir_path = os.path.dirname(APP_DRAW_FILE)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path, exist_ok=True)

        with open(APP_DRAW_FILE, "w", encoding="utf-8") as file:
            file.write(self._desktop_content)

        self._logger.debug(f"Menu entry file '{APP_DRAW_FILE}' written successfully")

    def relaunch_application(self) -> None:
        """Relaunch the application after 1 second"""
        NOTIFIER.show_toast(translator.translate("applying.update"))
        event_bus.emit("stop")
        subprocess.run(
            f'nohup bash -c "sleep 1 && {self._runner_file}" > /dev/null 2>&1 &',
            check=False,
            shell=True,
        )
        os.kill(os.getpid(), signal.SIGKILL)


application = Application()
