from rcc.clients.dbus.base.abstract_dbus_client import AbstractDbusClient
from rcc.utils.singleton import singleton
from rcc.clients.file.steam_client import steam_client  # pylint: disable=E0611,W0611


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
