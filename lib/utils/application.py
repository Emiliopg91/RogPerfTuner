from .. import __app_name__

from .constants import (
    autostart_file,
    app_draw_file,
    icons_path,
    user_icon_folder,
    dev_mode,
)
from .logger import Logger

import os
import shutil


class Application:
    def __init__(self):
        self.logger = Logger(self.__class__.__name__)
        self.desktop_content = f"""[Desktop Entry]
Exec={os.getenv("APPIMAGE")}
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


application = Application()
