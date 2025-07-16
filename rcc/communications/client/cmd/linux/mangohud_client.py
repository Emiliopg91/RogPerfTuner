from rcc.communications.client.cmd.abstract_cmd_client import AbstractCmdClient


class MangoHudClient(AbstractCmdClient):
    """Mango hud client"""

    def __init__(self):
        super().__init__("mangohud")


MANGO_HUD_CLIENT = MangoHudClient()
