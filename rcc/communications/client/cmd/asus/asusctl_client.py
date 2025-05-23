from rcc.communications.client.cmd.abstract_cmd_client import AbstractCmdClient


class AsusCtlClient(AbstractCmdClient):
    """Client for asusctl"""

    def __init__(self):
        super().__init__("asusctl")

    def set_aura(self, mode: str, color: str):
        """Set aura mode and color"""
        self._run_command(f"aura {mode} -c {color}")


ASUS_CTL_CLIENT = AsusCtlClient()
