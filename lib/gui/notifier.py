from ..utils.constants import icons_path
from ..utils.singleton import singleton

import os
import subprocess

class Notifier:

    def show_toast(self, message):
        command = f"notify-send \" \" \"{message}\" -a \"RogControlCenter\" -i {os.path.join(icons_path, "icon-45x45.png")}"
        subprocess.run(command, shell=True, check=False)

notifier = Notifier()