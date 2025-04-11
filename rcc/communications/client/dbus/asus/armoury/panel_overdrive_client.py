from framework.singleton import singleton
from rcc.communications.client.dbus.asus.armoury.armoury_base_client import ArmouryBaseClient


@singleton
class PanelOverdriveClient(ArmouryBaseClient):
    """DBus platform client"""

    def __init__(self):
        super().__init__("panel_overdrive", False)


PANEL_OVERDRIVE_CLIENT = PanelOverdriveClient()
