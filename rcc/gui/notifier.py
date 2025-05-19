import os

from rcc.communications.client.dbus.linux.notification_client import NOTIFICATION_CLIENT
from rcc.utils.constants import APP_NAME, ICONS_PATH
from framework.singleton import singleton


@singleton
class Notifier:
    """Class for showing notifications"""

    ICON_PATH = os.path.join(ICONS_PATH, "icon-45x45.png")

    def __init__(self):
        self.last_id = 0

    def show_toast(self, message, can_be_hidden=True, icon=ICON_PATH):
        """Show notification"""
        if NOTIFICATION_CLIENT.available:
            if self.last_id > 0:
                NOTIFICATION_CLIENT.close_notification(self.last_id)

            try:
                toast_id = NOTIFICATION_CLIENT.show_notification(" ", icon, APP_NAME, message + "\n\t", 3000)
                if can_be_hidden:
                    self.last_id = toast_id
            except Exception:
                """"""


NOTIFIER = Notifier()
