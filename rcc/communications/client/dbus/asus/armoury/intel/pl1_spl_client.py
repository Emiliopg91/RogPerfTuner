from framework.singleton import singleton
from rcc.communications.client.dbus.asus.armoury.armoury_base_client import ArmouryBaseClient


@singleton
class Pl1SplClient(ArmouryBaseClient):
    """DBus platform client"""

    def __init__(self):
        super().__init__("ppt_pl1_spl", False)


PL1_SPL_CLIENT = Pl1SplClient()
