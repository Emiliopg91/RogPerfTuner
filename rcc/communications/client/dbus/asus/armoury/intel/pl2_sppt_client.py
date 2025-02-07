from framework.singleton import singleton
from rcc.communications.client.dbus.asus.armoury.armoury_base_client import ArmouryBaseClient


@singleton
class Pl2SpptClient(ArmouryBaseClient):
    """DBus platform client"""

    def __init__(self):
        super().__init__("ppt_pl2_sppt", False)


PL2_SPPT_CLIENT = Pl2SpptClient()
