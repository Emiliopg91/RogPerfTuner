from rcc.clients.dbus.base.abstract_dbus_client import AbstractDbusClient
from rcc.utils.singleton import singleton


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

    @property
    def on_battery(self) -> bool:
        """Battery flag"""
        return self._get_property("OnBattery")


upower_client = UpowerClient()
