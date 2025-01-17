# pylint: disable=E0611, E0401
from PyQt6.QtCore import QMetaType
from PyQt6.QtDBus import QDBusArgument

from lib.clients.dbus.base.abstract_dbus_client import AbstractDbusClient
from lib.utils.singleton import singleton


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
            QDBusArgument(0, QMetaType.Type.UInt.value),
            app_icon,
            title,
            body,
            QDBusArgument([], QMetaType.Type.QStringList.value),
            {},
            timeout,
        )

    def close_notification(self, notif_id: int):
        """Close notification"""
        return self._invoke_method(
            "CloseNotification",
            QDBusArgument(notif_id, QMetaType.Type.UInt.value),
        )


notification_client = NotificationClient()
