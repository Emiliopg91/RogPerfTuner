from math import floor
import os
from threading import Lock, Thread
import time

from psutil import Process
import pyudev

from framework.logger import Logger
from rcc.communications.client.cmd.linux.lsusb_client import LS_USB_CLIENT
from rcc.communications.client.dbus.asus.armoury.intel.pl1_spl_client import PL1_SPL_CLIENT
from rcc.communications.client.dbus.asus.armoury.intel.pl2_sppt_client import PL2_SPPT_CLIENT
from rcc.communications.client.dbus.asus.armoury.nvidia.nv_boost_client import NV_BOOST_CLIENT
from rcc.communications.client.dbus.asus.armoury.nvidia.nv_temp_client import NV_TEMP_CLIENT
from rcc.communications.client.dbus.asus.armoury.panel_overdrive_client import PANEL_OVERDRIVE_CLIENT
from rcc.communications.client.dbus.asus.core.platform_client import PLATFORM_CLIENT
from rcc.communications.client.dbus.linux.power_management_keyboard_brightness_control import (
    KEYBOARD_BRIGHTNESS_CONTROL,
)
from rcc.communications.client.dbus.linux.switcheroo_client import SWITCHEROO_CLIENT
from rcc.communications.client.dbus.linux.upower_client import UPOWER_CLIENT
from rcc.communications.client.file.linux.boost_client import BOOST_CLIENT
from rcc.communications.client.file.linux.cpuinfo_client import CPU_INFO_CLIENT
from rcc.communications.client.file.linux.scheduler_client import SSD_SCHEDULER_CLIENT
from rcc.communications.client.tcp.openrgb.openrgb_client import OPEN_RGB_CLIENT
from rcc.gui.notifier import NOTIFIER
from rcc.models.battery_threshold import BatteryThreshold
from rcc.models.cpu_brand import CpuBrand
from rcc.models.gpu_brand import GpuBrand
from rcc.models.performance_profile import PerformanceProfile
from rcc.models.ssd_scheduler import SsdScheduler
from rcc.models.usb_identifier import UsbIdentifier
from rcc.utils.beans import EVENT_BUS, TRANSLATOR
from rcc.utils.events import (
    HARDWARE_SERVICE_BATTERY_THRESHOLD_CHANGED,
    HARDWARE_SERVICE_ON_BATTERY,
    HARDWARE_SERVICE_ON_USB_CHANGED,
    STEAM_SERVICE_GAME_EVENT,
)
from rcc.utils.shell import SHELL


class HardwareService:
    """Hardware service"""

    CPU_PRIORITY = -15
    IO_PRIORITY = floor((CPU_PRIORITY + 20) / 5)
    IO_CLASS = 2

    def __init__(self):  # pylint: disable=too-many-branches
        self._logger = Logger()
        self._logger.info("Initializing HardwareService")
        self._logger.add_tab()

        self._logger.info("Detecting CPU")
        self._logger.add_tab()
        self.__cpu = None
        output = CPU_INFO_CLIENT.read(5)
        if "GenuineIntel" in output:
            self.__cpu = CpuBrand.INTEL
            line = output.splitlines()[-1]
            line = line[line.index(":") + 1 :].strip()
            self._logger.info(line)
        self._logger.add_tab()

        if self.__cpu == CpuBrand.INTEL:
            if PL1_SPL_CLIENT.available:
                self._logger.info("TDP control available")

            if BOOST_CLIENT.available:
                self._logger.info("Boost control available")

        self._logger.rem_tab()
        self._logger.rem_tab()

        self.__gpus = {}
        if SWITCHEROO_CLIENT.available:  # pylint: disable=too-many-nested-blocks
            self._logger.info("Detecting GPU")
            self._logger.add_tab()
            gpus = list(SWITCHEROO_CLIENT.gpus)
            for gpu in sorted(gpus, key=lambda x: (x["Name"])):
                self._logger.info(f"{gpu["Name"]}")
                if gpu["Discrete"]:
                    brand = GpuBrand(gpu["Name"].split(" ")[0].lower())
                    env = None
                    if len(gpu["Environment"]) > 1:
                        env = ""
                        for i in range(0, len(gpu["Environment"]), 2):
                            env += gpu["Environment"][i] + "=" + gpu["Environment"][i + 1] + " "
                        env = env.strip()

                    self.__gpus[brand] = env
                    self._logger.add_tab()

                    if brand == GpuBrand.NVIDIA:
                        if NV_BOOST_CLIENT.available:
                            self._logger.info("Dynamic boost control available")
                        if NV_TEMP_CLIENT.available:
                            self._logger.info("Throttle temperature control available")

                    self._logger.rem_tab()

        self._logger.rem_tab()

        if SSD_SCHEDULER_CLIENT.available:
            self._logger.info("Getting available SSD schedulers")
            self._logger.add_tab()
            self.__available_ssd_sched = SSD_SCHEDULER_CLIENT.get_schedulers()
            for sched in self.__available_ssd_sched:
                self._logger.info(f"{sched.name} - {sched.value}")
            self._logger.rem_tab()

        if UPOWER_CLIENT.available:
            self.__on_bat = UPOWER_CLIENT.on_battery
            UPOWER_CLIENT.on_battery_change(self._on_ac_battery_change)

        if PLATFORM_CLIENT.available:
            self.__battery_charge_limit = PLATFORM_CLIENT.charge_control_end_threshold

        self.__running_games = 0

        self._connected_usb: list[UsbIdentifier] = []
        self._usb_mutex = Lock()
        thread = Thread(name="UsbChecker", target=self.__monitor_for_usb)
        thread.start()

        if KEYBOARD_BRIGHTNESS_CONTROL.available:
            KEYBOARD_BRIGHTNESS_CONTROL.on_brightness_change(self._on_kb_brightness_change)

        EVENT_BUS.on(STEAM_SERVICE_GAME_EVENT, self.__on_game_event)

        self._logger.rem_tab()

    def _on_kb_brightness_change(self, value: int):
        if value == 0 and KEYBOARD_BRIGHTNESS_CONTROL.available:
            KEYBOARD_BRIGHTNESS_CONTROL.keyboard_brightness = 2

    def get_gpu_selector_env(self, gpu: GpuBrand):
        """Get ENV configuration for GPU selection"""
        env = f"RCC_GPU={gpu.value} "

        icds = [
            icd
            for icd in [
                f"/usr/share/vulkan/icd.d/{gpu.value}_icd.i686.json",
                f"/usr/share/vulkan/icd.d/{gpu.value}_icd.x86_64.json",
            ]
            if os.path.exists(icd)
        ]
        if len(icds) > 0:
            env += f"VK_ICD_FILENAMES={":".join(icds)} "

        ocds = [ocd for ocd in [f"/etc/OpenCL/vendors/{gpu.value}.icd"] if os.path.exists(ocd)]
        if len(ocds) > 0:
            env += f"OCL_ICD_FILENAMES={":".join(ocds)} "

        if gpu in self.__gpus and self.__gpus[gpu] is not None:
            env += self.__gpus[gpu]

        return env.strip()

    @property
    def gpus(self) -> list[GpuBrand]:
        """GPU brand"""
        return self.__gpus.keys()

    @property
    def cpu(self) -> CpuBrand | None:
        """CPU brand"""
        return self.__cpu

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
        if PLATFORM_CLIENT.available:
            if value != self.__battery_charge_limit:
                PLATFORM_CLIENT.charge_control_end_threshold = value
                self.__battery_charge_limit = value
                EVENT_BUS.emit(HARDWARE_SERVICE_BATTERY_THRESHOLD_CHANGED, value)
                NOTIFIER.show_toast(TRANSLATOR.translate("applied.battery.threshold", {"value": value.value}))

    def set_ssd_scheduler(self, scheduler: SsdScheduler):
        """Set SSD queue scheduler"""
        if SSD_SCHEDULER_CLIENT.available and scheduler in self.__available_ssd_sched:
            self._logger.info(f"SSD scheduler: {scheduler.name}")
            SSD_SCHEDULER_CLIENT.set_scheduler(scheduler)

    def set_boost_status(self, enabled: bool):
        """Enable/disable cpu boost"""
        if BOOST_CLIENT.available:
            self._logger.info(f"CPU boost: {"ENABLED" if enabled else "DISABLED"}")
            BOOST_CLIENT.boost = enabled

    def __monitor_for_usb(self) -> None:  # pylint: disable=too-many-locals, too-many-branches
        """Monitor for usb devices changes"""
        monitor = pyudev.Monitor.from_netlink(pyudev.Context())
        monitor.filter_by("usb")

        lsusb_output = LS_USB_CLIENT.get_usb_dev()
        current_usb = []
        for line in lsusb_output.split("\n"):
            columns = line.strip().split(" ")

            id_vendor, id_product = columns[5].split(":")
            name = " ".join(columns[6:])

            usb_dev = UsbIdentifier(id_vendor, id_product, name)

            for cd in OPEN_RGB_CLIENT.compatible_devices:
                if cd.id_vendor == usb_dev.id_vendor and cd.id_product == usb_dev.id_product:
                    self._connected_usb.append(cd)

        for action, _ in monitor:  # pylint: disable=too-many-nested-blocks
            if action in ["add", "remove"]:
                self._usb_mutex.acquire(True)  # pylint: disable=consider-using-with
                try:
                    lsusb_output = LS_USB_CLIENT.get_usb_dev()

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
        if PANEL_OVERDRIVE_CLIENT.available:
            self._logger.info(f"Setting panel overdrive {"disabled" if not enabled else "enabled"}")
            PANEL_OVERDRIVE_CLIENT.current_value = 1 if enabled else 0

    def apply_proccess_optimizations(self, pid: int, first_run=True):
        """Apply optimizations to the process tree"""
        if first_run:
            self._logger.info(
                f"Setting CPU priority to {self.CPU_PRIORITY} and " + f"IO priority to {self.IO_PRIORITY}"
            )

        pending = [pid]
        processed = []

        while True:
            new_pids_found = False

            while pending:
                pid = pending.pop(0)
                if pid not in processed:
                    try:
                        SHELL.run_command(
                            f"renice -n {self.CPU_PRIORITY} -p {pid} && "
                            + f"ionice -c {self.IO_CLASS} -n {self.IO_PRIORITY} -p {pid}",
                            sudo=True,
                            check=True,
                        )
                    except Exception as e:
                        self._logger.debug(f"Could not apply optimizations for process {pid}: {e}")
                    processed.append(pid)

                try:
                    for child in Process(pid).children():
                        if child.pid not in pending and child.pid not in processed:
                            pending.append(child.pid)
                            new_pids_found = True
                except Exception:
                    pass

            if not new_pids_found:
                break

            pending = processed
            processed = []

            time.sleep(0.05)

        if first_run:
            self._logger.info(f"Optimized {len(processed)} processes")

        return len(processed)

    def set_cpu_tdp(self, profile: PerformanceProfile):
        """Set CPU TDP configuration"""
        if HARDWARE_SERVICE.cpu == CpuBrand.INTEL:
            pl1 = profile.ac_intel_pl1_spl
            pl2 = profile.ac_intel_pl2_sppt
            if pl1 is not None:
                time.sleep(0.05)
                self._logger.info(f"{self.cpu.capitalize()} CPU")

                if UPOWER_CLIENT.available and UPOWER_CLIENT.on_battery:
                    pl1 = profile.battery_intel_pl1_spl
                    pl2 = profile.battery_intel_pl2_sppt

                self._logger.info(f"  PL1: {pl1}W")
                PL1_SPL_CLIENT.current_value = PL1_SPL_CLIENT.default_value
                time.sleep(0.05)
                PL1_SPL_CLIENT.current_value = pl1

                if pl2 is not None:
                    self._logger.info(f"  PL2: {pl2}W")
                    PL2_SPPT_CLIENT.current_value = PL2_SPPT_CLIENT.default_value
                    time.sleep(0.05)
                    PL2_SPPT_CLIENT.current_value = pl2

    def set_gpu_tgp(self, profile: PerformanceProfile):
        """Set GPU TGP parameters"""
        if GpuBrand.NVIDIA in self.__gpus:
            nv = profile.ac_nv_boost
            nt = profile.ac_nv_temp
            if nv is not None or nt is not None:
                self._logger.info("Nvidia GPU")
                if UPOWER_CLIENT.available and UPOWER_CLIENT.on_battery:
                    nv = profile.battery_nv_boost
                    nt = profile.battery_nv_temp

                if nv is not None:
                    self._logger.info(f"  Dynamic Boost: {nv}W")
                    NV_BOOST_CLIENT.current_value = nv

                if nt is not None:
                    self._logger.info(f"  Throttle temp: {nt}ºC")
                    NV_TEMP_CLIENT.current_value = nt

    @property
    def tdp_available(self):
        """Flag for TDP support"""
        return (GpuBrand.NVIDIA in self.__gpus and (NV_BOOST_CLIENT.available or NV_TEMP_CLIENT.available)) or (
            HARDWARE_SERVICE.cpu == CpuBrand.INTEL and PL1_SPL_CLIENT.available
        )


HARDWARE_SERVICE = HardwareService()
