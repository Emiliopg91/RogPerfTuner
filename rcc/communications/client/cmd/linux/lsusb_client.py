from typing import Callable
from rcc.communications.client.cmd.abstract_cmd_client import AbstractCmdClient
from rcc.models.usb_identifier import UsbIdentifier


class LsUsbClient(AbstractCmdClient):
    """lsusb client"""

    def __init__(self):
        super().__init__("lsusb")

    def get_usb_dev(self):
        """Get connected devices"""
        return self._run_command()[1].strip()

    def compare_connected_devs(
        self, previous: list[UsbIdentifier], dev_filter: Callable[[UsbIdentifier], bool] = None
    ):  # pylint: disable=too-many-locals
        """Get current, added and removed usb devices"""
        lsusb_output = self.get_usb_dev()

        current_usb: list[UsbIdentifier] = []
        removed: list[UsbIdentifier] = []
        added: list[UsbIdentifier] = []

        for line in lsusb_output.split("\n"):
            columns = line.strip().split(" ")

            id_vendor, id_product = columns[5].split(":")
            name = " ".join(columns[6:])

            usb_dev = UsbIdentifier(id_vendor, id_product, name)
            if dev_filter is None or dev_filter(usb_dev):
                current_usb.append(usb_dev)

        for dev1 in current_usb:
            found = False
            for dev2 in previous:
                if not found and dev1.id_vendor == dev2.id_vendor and dev1.id_product == dev2.id_product:
                    found = True
            if not found:
                added.append(dev1)

        for dev1 in previous:
            found = False
            for dev2 in current_usb:
                if not found and dev1.id_vendor == dev2.id_vendor and dev1.id_product == dev2.id_product:
                    found = True
            if not found:
                removed.append(dev1)

        return (current_usb, added, removed)


LS_USB_CLIENT = LsUsbClient()
