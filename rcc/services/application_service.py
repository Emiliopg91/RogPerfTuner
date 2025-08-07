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

    RUNNER_FILE_PATH = os.path.join(USER_BIN_FOLDER, "application", "launch.sh")
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

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing ApplicationService")
        self._rccdc_enabled = False

        shutil.copy2(
            os.path.join(ICONS_PATH, "rog-logo.svg"),
            os.path.join(USER_ICON_FOLDER, "icon.svg"),
        )

    @property
    def is_autostart(self):
        """Flag for autostart state"""
        return os.path.exists(AUTOSTART_FILE)

    def enable_autostart(self, enable) -> None:
        """Create file to enable autostart on login"""
        self._logger.info("Setting autostart")
        self._logger.add_tab()

        if (enable and self.is_autostart) or (not enable and not self.is_autostart):
            self._logger.info("Autostart state not changed")
        else:
            if enable:
                dir_path = os.path.dirname(AUTOSTART_FILE)
                if not os.path.exists(dir_path):
                    os.makedirs(dir_path, exist_ok=True)
                with open(AUTOSTART_FILE, "w", encoding="utf-8") as file:
                    file.write(self.DESKTOP_FILE_CONTENT)
                self._logger.info(f"Autostart file '{AUTOSTART_FILE}' written successfully")
            else:
                os.unlink(AUTOSTART_FILE)
                self._logger.info(f"Autostart file '{AUTOSTART_FILE}' deleted successfully")

        self._logger.rem_tab()

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
        SHELL.run_command(f'nohup bash -c "sleep 1 && {self.RUNNER_FILE_PATH}" > /dev/null 2>&1 &')
        os.kill(os.getpid(), signal.SIGKILL)

    def generate_run(self) -> None:
        """Generate runner file"""
        SHELL.copy_scripts(
            "application",
            {
                "update": self.UPDATE_FILE_PATH,
                "appimage": os.getenv("APPIMAGE"),
            },
        )

        self._logger.debug(f"Launch file '{self.RUNNER_FILE_PATH}' written successfully")


APPLICATION_SERVICE = ApplicationService()
