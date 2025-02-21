from threading import Lock, Thread

import concurrent
import pyudev
from framework.logger import Logger
from rcc.communications.client.dbus.asus.armoury.intel.pl1_spl_client import PL1_SPL_CLIENT
from rcc.communications.client.dbus.asus.armoury.nvidia.nv_boost_client import NV_BOOST_CLIENT
from rcc.communications.client.dbus.asus.armoury.panel_overdrive_client import PANEL_OVERDRIVE_CLIENT
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
from rcc.utils.shell import SHELL


class HardwareService:
    """Hardware service"""

    CPU_PRIORITY = -10
    IO_PRIORITY = int((CPU_PRIORITY + 20) / 5)
    IO_CLASS = 2

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing HardwareService")
        self._logger.add_tab()

        if PL1_SPL_CLIENT.available:
            self._logger.info("Detected Intel CPU")
            self.__hp_cores = None
            self.__determine_cpu_architecture()

        if NV_BOOST_CLIENT.available:
            self._logger.info("Detected Nvidia GPU")

        self.__on_bat = UPOWER_CLIENT.on_battery
        self.__battery_charge_limit = PLATFORM_CLIENT.charge_control_end_threshold
        self.__running_games = 0

        self._connected_usb: list[UsbIdentifier] = []
        self._usb_mutex = Lock()
        thread = Thread(name="UsbChecker", target=self.__monitor_for_usb)
        thread.start()

        UPOWER_CLIENT.on_battery_change(self._on_ac_battery_change)
        EVENT_BUS.on(STEAM_SERVICE_GAME_EVENT, self.__on_game_event)

        self._logger.rem_tab()

    def __determine_cpu_architecture(self):
        output = SHELL.run_command("lscpu -e", output=True)[1]
        lines = output.splitlines()[1:]

        found_cores: dict[int, list[int]] = {}

        for line in lines:
            cpu = int(line[0:3].strip())
            core = int(line[15:20].strip())
            if core not in found_cores:
                found_cores[core] = []
            found_cores[core].append(cpu)

        p_cores = []
        e_cores = []
        for core, cores_list in found_cores.items():
            if len(cores_list) > 1:
                p_cores.extend(cores_list)
            else:
                e_cores.append(cores_list[0])

        if len(p_cores) > 0 and len(e_cores) > 0:
            self.__hp_cores = p_cores
            self._logger.info(f"Hybrid CPU found. High performance cores: {self.__hp_cores[0]}-{self.__hp_cores[-1]}")

    @property
    def gpu(self):
        """dGPU brand"""
        if NV_BOOST_CLIENT.available:
            return "nvidia"

        return None

    @property
    def hp_cores(self):
        """High performance cores"""
        return self.__hp_cores

    def _on_ac_battery_change(self, on_battery: bool, muted=False):
        self.__on_bat = on_battery
        if self.__running_games == 0:
            self.set_panel_overdrive(self.__running_games > 0 and not self.__on_bat)

        if self.__running_games == 0:
            if not muted:
                self._logger.info(
                    f"AC {"un" if on_battery else ""}plugged, battery {"dis" if not on_battery else ""}engaged"
                )
                self._logger.add_tab()
            EVENT_BUS.emit_sequencial(HARDWARE_SERVICE_ON_BATTERY, on_battery)
            if not muted:
                self._logger.rem_tab()

    def __on_game_event(self, count):
        self.__running_games = count

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

        lsusb_output = SHELL.run_command("lsusb", output=True)[1].strip()
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
                    lsusb_output = SHELL.run_command("lsusb", output=True)[1].strip()

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

    def set_panel_overdrive(self, enabled):
        """Enable or disable panel overdrive"""
        self._logger.info(f"Setting panel overdrive {"disabled" if not enabled else "enabled"}")
        if PANEL_OVERDRIVE_CLIENT.available:
            PANEL_OVERDRIVE_CLIENT.current_value = 1 if enabled else 0

    def apply_process_optimizations(self, pids):
        """Change CPU and IO nice value"""
        self._logger.info(
            f"Setting CPU affinity to cores {self.__hp_cores[0]}-{self.__hp_cores[-1]}, "
            + f"priority to {self.CPU_PRIORITY} and "
            + f"IO priority to {self.IO_PRIORITY}"
        )

        with concurrent.futures.ThreadPoolExecutor() as executor:
            futures = []
            for pid in pids:
                futures.append(executor.submit(lambda pid=pid: self.__apply_affinity(pid)))
                futures.append(executor.submit(lambda pid=pid: self.__apply_priority(pid)))
            concurrent.futures.wait(futures)

    def __apply_affinity(self, pid):
        try:
            SHELL.run_command(
                f"taskset -cp {self.__hp_cores[0]}-{self.__hp_cores[-1]} {pid}",
                sudo=True,
                check=True,
            )
        except Exception as e:
            self._logger.error(f"Could not set affinity of process {pid}: {e}")

    def __apply_priority(self, pid):
        try:
            SHELL.run_command(
                f"renice -n {self.CPU_PRIORITY} -p {pid} && "
                + f"ionice -c {self.IO_CLASS} -n {self.IO_PRIORITY} -p {pid}",
                sudo=True,
                check=True,
            )
        except Exception as e:
            self._logger.error(f"Could not set affinity of process {pid}: {e}")


HARDWARE_SERVICE = HardwareService()
