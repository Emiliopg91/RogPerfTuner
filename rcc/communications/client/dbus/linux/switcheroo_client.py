from rcc.communications.client.dbus.abstract_dbus_client import AbstractDbusClient
from framework.singleton import singleton


@singleton
class SwitcherooClient(AbstractDbusClient):
    """Dbus upower client"""

    def __init__(self):
        super().__init__(
            True,
            "net.hadess.SwitcherooControl",
            "/net/hadess/SwitcherooControl",
            "net.hadess.SwitcherooControl",
            required=False,
        )

    @property
    def gpus(self) -> bool:
        """Battery flag"""
        return self._get_property("GPUs")


SWITCHEROO_CLIENT = SwitcherooClient()
