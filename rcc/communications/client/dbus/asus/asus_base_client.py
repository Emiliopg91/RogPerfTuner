from rcc.communications.client.dbus.abstract_dbus_client import AbstractDbusClient


class AsusBaseClient(AbstractDbusClient):
    """DBus platform client"""

    def __init__(self, interface_sufix: str, object_path_sufix: str = "", required=True):  # pylint: disable=R0913,R0917
        object_sufix = object_path_sufix
        if object_sufix != "":
            object_sufix = "/" + object_sufix

        super().__init__(
            True,
            "xyz.ljones.Asusd",
            "/xyz/ljones" + object_sufix,
            "xyz.ljones." + interface_sufix,
            required,
        )
