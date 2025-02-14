import subprocess
from threading import Lock, Thread

import pyudev
from framework.logger import Logger
from rcc.communications.client.dbus.asus.core.platform_client import PLATFORM_CLIENT
from rcc.communications.client.dbus.linux.upower_client import UPOWER_CLIENT
from rcc.communications.client.tcp.openrgb.openrgb_client import OPEN_RGB_CLIENT
from rcc.gui.notifier import NOTIFIER
from rcc.models.battery_threshold import BatteryThreshold
from rcc.models.usb_identifier import UsbIdentifier
from rcc.utils.beans import EVENT_BUS, TRANSLATOR
from rcc.utils.events import (
    HARDWARE_SERVICE_ON_BATTERY,
    HARDWARE_SERVICE_BATTERY_THRESHOLD_CHANGED,
    HARDWARE_SERVICE_ON_USB_CHANGED,
    STEAM_SERVICE_GAME_EVENT,
)


class HardwareService:
    """Hardware service"""

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing HardwareService")

        self.__on_bat = UPOWER_CLIENT.on_battery
        self.__battery_charge_limit = PLATFORM_CLIENT.charge_control_end_threshold
        self.__ac_events_enabled = True

        self._connected_usb: list[UsbIdentifier] = []
        self._usb_mutex = Lock()
        thread = Thread(name="UsbChecker", target=self.__monitor_for_usb)
        thread.start()

        UPOWER_CLIENT.on_battery_change(self._on_ac_battery_change)
        EVENT_BUS.on(STEAM_SERVICE_GAME_EVENT, self.__on_game_event)

    def _on_ac_battery_change(self, on_battery: bool, muted=False):
        self.__on_bat = on_battery
        if self.__ac_events_enabled:
            if not muted:
                self._logger.info(
                    f"AC cord {"dis" if on_battery else ""}connected, battery {"dis" if not on_battery else ""}engaged"
                )
                self._logger.add_tab()
            EVENT_BUS.emit_sequencial(HARDWARE_SERVICE_ON_BATTERY, on_battery)
            if not muted:
                self._logger.rem_tab()

    def __on_game_event(self, count):
        self.__ac_events_enabled = count == 0

    @property
    def on_battery(self):
        """On battery flag"""
        return self.__on_bat

    @property
    def battery_charge_limit(self) -> BatteryThreshold:
        """Get current battery charge limit"""
        return self.__battery_charge_limit

    def set_battery_threshold(self, value: BatteryThreshold) -> None:
        """Set battery charge threshold"""
        if value != self.__battery_charge_limit:
            PLATFORM_CLIENT.charge_control_end_threshold = value
            self.__battery_charge_limit = value
            EVENT_BUS.emit(HARDWARE_SERVICE_BATTERY_THRESHOLD_CHANGED, value)
            NOTIFIER.show_toast(TRANSLATOR.translate("applied.battery.threshold", {"value": value.value}))

    def __monitor_for_usb(self) -> None:  # pylint: disable=R0914, R0912
        """Monitor for usb devices changes"""
        monitor = pyudev.Monitor.from_netlink(pyudev.Context())
        monitor.filter_by("usb")

        lsusb_output = subprocess.check_output(["lsusb"]).decode("utf-8").strip()
        current_usb = []
        for line in lsusb_output.split("\n"):
            columns = line.strip().split(" ")

            id_vendor, id_product = columns[5].split(":")
            name = " ".join(columns[6:])

            usb_dev = UsbIdentifier(id_vendor, id_product, name)

            for cd in OPEN_RGB_CLIENT.compatible_devices:
                if cd.id_vendor == usb_dev.id_vendor and cd.id_product == usb_dev.id_product:
                    self._connected_usb.append(cd)

        for action, _ in monitor:  # pylint: disable=R1702
            if action in ["add", "remove"]:
                self._usb_mutex.acquire(True)  # pylint: disable=R1732
                try:
                    lsusb_output = subprocess.check_output(["lsusb"]).decode("utf-8").strip()

                    current_usb = []
                    for line in lsusb_output.split("\n"):
                        columns = line.strip().split(" ")

                        id_vendor, id_product = columns[5].split(":")
                        name = " ".join(columns[6:])

                        usb_dev = UsbIdentifier(id_vendor, id_product, name)

                        if any(
                            cd.id_vendor == usb_dev.id_vendor and cd.id_product == usb_dev.id_product
                            for cd in OPEN_RGB_CLIENT.compatible_devices
                        ):
                            current_usb.append(usb_dev)

                    added = []
                    for dev1 in current_usb:
                        found = False
                        for dev2 in self._connected_usb:
                            if not found and dev1.id_vendor == dev2.id_vendor and dev1.id_product == dev2.id_product:
                                found = True
                        if not found:
                            added.append(dev1)

                    removed = []
                    for dev1 in self._connected_usb:
                        found = False
                        for dev2 in current_usb:
                            if not found and dev1.id_vendor == dev2.id_vendor and dev1.id_product == dev2.id_product:
                                found = True
                        if not found:
                            removed.append(dev1)

                    if len(removed) > 0:
                        self._logger.info("Removed compatible device(s):")
                        self._logger.add_tab()
                        for item in removed:
                            self._logger.info(OPEN_RGB_CLIENT.get_device_name(item.id_vendor, item.id_product))
                            OPEN_RGB_CLIENT.disable_device(item.name)
                        self._logger.rem_tab()

                    if len(added) > 0:
                        self._logger.info("Connected compatible device(s):")
                        self._logger.add_tab()
                        for item in added:
                            self._logger.info(OPEN_RGB_CLIENT.get_device_name(item.id_vendor, item.id_product))
                        self._logger.rem_tab()

                        self._logger.add_tab()
                        EVENT_BUS.emit_sequencial(HARDWARE_SERVICE_ON_USB_CHANGED)
                        self._logger.rem_tab()
                        self._usb_mutex.release()
                    else:
                        self._usb_mutex.release()

                    self._connected_usb = current_usb
                except Exception:
                    self._usb_mutex.release()


HARDWARE_SERVICE = HardwareService()
