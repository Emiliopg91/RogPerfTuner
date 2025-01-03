import os
import subprocess

from lib.utils.constants import icons_path
from lib.utils.singleton import singleton


@singleton
class Notifier:
    """Class for showing notifications"""

    def show_toast(self, message):
        """Show notification"""
        command = (
            f'notify-send " " "{message}" -a "RogControlCenter" -t 3000 -i {os.path.join(icons_path, "icon-45x45.png")}'
        )
        subprocess.run(command, shell=True, check=False)


notifier = Notifier()
