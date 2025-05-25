from rcc.communications.client.cmd.abstract_cmd_client import AbstractCmdClient


class LsUsbClient(AbstractCmdClient):
    """lsusb client"""

    def __init__(self):
        super().__init__("lsusb")

    def get_usb_dev(self):
        """Get connected devices"""
        return self._run_command()[1].strip()


LS_USB_CLIENT = LsUsbClient()
