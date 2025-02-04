from PyQt5.QtCore import QMetaType
from PyQt5.QtDBus import QDBusArgument

from rcc.communications.client.dbus.base.abstract_dbus_client import AbstractDbusClient
from framework.singleton import singleton


@singleton
class NotificationClient(AbstractDbusClient):
    """Notification1 client"""

    def __init__(self):
        super().__init__(
            False, "org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications"
        )

    def show_notification(  # pylint: disable=R0913,R0917
        self, app_name: str, app_icon: str, title: str, body: str, timeout: int
    ) -> int:
        """Show notification"""
        return self._invoke_method(
            "Notify",
            app_name,
            QDBusArgument(0, QMetaType.UInt),
            app_icon,
            title,
            body,
            QDBusArgument([], QMetaType.QStringList),
            {},
            timeout,
        )

    def close_notification(self, notif_id: int):
        """Close notification"""
        return self._invoke_method(
            "CloseNotification",
            QDBusArgument(notif_id, QMetaType.UInt),
        )


NOTIFICATION_CLIENT = NotificationClient()
