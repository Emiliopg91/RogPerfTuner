from lib.clients.dbus.base.abstract_dbus_client import AbstractDbusClient
from lib.utils.singleton import singleton


@singleton
class UpowerClient(AbstractDbusClient):
    """Dbus upower client"""

    def __init__(self):
        super().__init__(
            True,
            "org.freedesktop.UPower",
            "/org/freedesktop/UPower",
            "org.freedesktop.UPower",
        )


upower_client = UpowerClient()
