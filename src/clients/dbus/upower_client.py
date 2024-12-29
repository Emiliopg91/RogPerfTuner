from .base.abstract_dbus_client import AbstractDbusClient
from ...utils.singleton import singleton


@singleton
class UpowerClient(AbstractDbusClient):
    def __init__(self):
        super().__init__(
            True,
            "org.freedesktop.UPower",
            "/org/freedesktop/UPower",
            "org.freedesktop.UPower",
        )


upower_client = UpowerClient()
