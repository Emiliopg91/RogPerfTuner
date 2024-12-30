from .. import __app_name__

from .constants import (
    autostart_file,
    app_draw_file,
    icons_path,
    user_icon_folder,
    user_bin_folder,
    user_update_folder,
)
from .event_bus import event_bus
from .logger import Logger
from .translator import translator
from ..gui.notifier import notifier

import os
import shutil
import signal
import subprocess


class Application:
    def __init__(self):
        self.logger = Logger(self.__class__.__name__)
        self.runner_file = os.path.join(user_bin_folder, "launch.sh")

        self.desktop_content = f"""[Desktop Entry]
Exec={self.runner_file}
Icon={os.path.join(user_icon_folder, "icon.svg")}
Name={__app_name__}
Comment=An utility to manage Asus Rog laptop performance
Path=
Terminal=False
Type=Application
    """
        shutil.copy2(
            os.path.join(icons_path, "rog-logo.svg"),
            os.path.join(user_icon_folder, "icon.svg"),
        )

    def generate_run(self):
        update_file = f"{os.path.join(user_update_folder, __app_name__)}.AppImage"
        content = f"""#!/bin/bash

if [[ -f "{update_file}" ]]; then
    cp "{update_file}" "{os.getenv("APPIMAGE")}"
    chmod 755 "{os.getenv("APPIMAGE")}"
fi

"{os.getenv("APPIMAGE")}"
"""
        with open(self.runner_file, "w", encoding="utf-8") as file:
            file.write(content)
        os.chmod(self.runner_file, 0o755)

        self.logger.info(f"Launch file '{self.runner_file}' written successfully")

    def enable_autostart(self):
        self.logger.info("Creating autostart file")
        dir_path = os.path.dirname(autostart_file)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path, exist_ok=True)

        with open(autostart_file, "w", encoding="utf-8") as file:
            file.write(self.desktop_content)

        self.logger.info(f"Autostart file '{autostart_file}' written successfully")

    def create_menu_entry(self):
        self.logger.info("Creating app menu file")
        dir_path = os.path.dirname(app_draw_file)
        if not os.path.exists(dir_path):
            os.makedirs(dir_path, exist_ok=True)

        with open(app_draw_file, "w", encoding="utf-8") as file:
            file.write(self.desktop_content)

        self.logger.info(f"Menu entry file '{app_draw_file}' written successfully")

    def relaunch_application(self):
        notifier.show_toast(translator.translate("applying.update"))
        event_bus.emit("stop")
        subprocess.run(
            f'nohup bash -c "sleep 1 && {self.runner_file}" > /dev/null 2>&1 &',
            shell=True,
        )
        os.kill(os.getpid(), signal.SIGKILL)


application = Application()
