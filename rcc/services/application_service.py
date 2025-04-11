import os
import shutil
import signal

from rcc.gui.notifier import NOTIFIER
from rcc.utils.beans import TRANSLATOR
from rcc.utils.constants import (
    APP_NAME,
    APP_DRAW_FILE,
    AUTOSTART_FILE,
    ICONS_PATH,
    USER_BIN_FOLDER,
    USER_ICON_FOLDER,
    USER_UPDATE_FOLDER,
)
from rcc.utils.beans import EVENT_BUS
from rcc.utils.shell import SHELL
from framework.logger import Logger

from framework.singleton import singleton


@singleton
class ApplicationService:
    """Class for managing applicaion aspects"""

    RUNNER_FILE_PATH = os.path.join(USER_BIN_FOLDER, "launch.sh")
    UPDATE_FILE_PATH = f"{os.path.join(USER_UPDATE_FOLDER, APP_NAME)}.AppImage"

    DESKTOP_FILE_CONTENT = f"""[Desktop Entry]
Exec={RUNNER_FILE_PATH}
Icon={os.path.join(USER_ICON_FOLDER, "icon.svg")}
Name={APP_NAME}
Comment=An utility to manage Asus Rog laptop performance
Path=
Terminal=False
Type=Application
Categories=Utility;
    """

    RUNNER_FILE_CONTENT = f"""#!/bin/bash
if [[ -f "{UPDATE_FILE_PATH}" ]]; then
    cp "{UPDATE_FILE_PATH}" "{os.getenv("APPIMAGE")}"
    chmod 755 "{os.getenv("APPIMAGE")}"
fi
"{os.getenv("APPIMAGE")}"
"""

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing ApplicationService")
        self._rccdc_enabled = False

        shutil.copy2(
            os.path.join(ICONS_PATH, "rog-logo.svg"),
            os.path.join(USER_ICON_FOLDER, "icon.svg"),
        )

    def enable_autostart(self) -> None:
        """Create file to enable autostart on login"""
        self._logger.debug("Creating autostart file")
        dir_path = os.path.dirname(AUTOSTART_FILE)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path, exist_ok=True)

        with open(AUTOSTART_FILE, "w", encoding="utf-8") as file:
            file.write(self.DESKTOP_FILE_CONTENT)

        self._logger.debug(f"Autostart file '{AUTOSTART_FILE}' written successfully")

    def create_menu_entry(self) -> None:
        """Create file to add application to menu"""
        self._logger.debug("Creating app menu file")
        dir_path = os.path.dirname(APP_DRAW_FILE)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path, exist_ok=True)

        with open(APP_DRAW_FILE, "w", encoding="utf-8") as file:
            file.write(self.DESKTOP_FILE_CONTENT)

        self._logger.debug(f"Menu entry file '{APP_DRAW_FILE}' written successfully")

    def relaunch_application(self) -> None:
        """Relaunch the application after 1 second"""
        NOTIFIER.show_toast(TRANSLATOR.translate("applying.update"))
        EVENT_BUS.emit("stop")
        SHELL.run_command(f'nohup bash -c "sleep 1 && {self.RUNNER_FILE_PATH}" > /dev/null 2>&1 &', check=False)
        os.kill(os.getpid(), signal.SIGKILL)

    def generate_run(self) -> None:
        """Generate runner file"""

        with open(self.RUNNER_FILE_PATH, "w", encoding="utf-8") as file:
            file.write(self.RUNNER_FILE_CONTENT)
        os.chmod(self.RUNNER_FILE_PATH, 0o755)

        self._logger.debug(f"Launch file '{self.RUNNER_FILE_PATH}' written successfully")


APPLICATION_SERVICE = ApplicationService()
