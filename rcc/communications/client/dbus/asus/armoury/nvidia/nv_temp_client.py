from framework.singleton import singleton
from rcc.communications.client.dbus.asus.armoury.armoury_base_client import ArmouryBaseClient


@singleton
class NvTempClient(ArmouryBaseClient):
    """DBus platform client"""

    def __init__(self):
        super().__init__("nv_temp_target", False)


NV_TEMP_CLIENT = NvTempClient()
