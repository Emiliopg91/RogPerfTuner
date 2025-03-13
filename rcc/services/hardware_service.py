from math import floor
import os
from threading import Lock, Thread
import time
from typing import Callable, Optional, Dict, List

from psutil import Process
import pyudev
from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

from framework.logger import Logger
from rcc.communications.client.dbus.asus.armoury.intel.pl1_spl_client import PL1_SPL_CLIENT
from rcc.communications.client.dbus.asus.armoury.intel.pl2_sppt_client import PL2_SPPT_CLIENT
from rcc.communications.client.dbus.asus.armoury.nvidia.nv_boost_client import NV_BOOST_CLIENT
from rcc.communications.client.dbus.asus.armoury.nvidia.nv_temp_client import NV_TEMP_CLIENT
from rcc.communications.client.dbus.asus.armoury.panel_overdrive_client import PANEL_OVERDRIVE_CLIENT
from rcc.communications.client.dbus.asus.core.platform_client import PLATFORM_CLIENT
from rcc.communications.client.dbus.linux.switcheroo_client import SWITCHEROO_CLIENT
from rcc.communications.client.dbus.linux.upower_client import UPOWER_CLIENT
from rcc.communications.client.tcp.openrgb.openrgb_client import OPEN_RGB_CLIENT
from rcc.gui.notifier import NOTIFIER
from rcc.models.battery_threshold import BatteryThreshold
from rcc.models.cpu_brand import CpuBrand
from rcc.models.gpu_brand import GpuBrand
from rcc.models.performance_profile import PerformanceProfile
from rcc.models.usb_identifier import UsbIdentifier
from rcc.utils.beans import EVENT_BUS, TRANSLATOR
from rcc.utils.events import (
    HARDWARE_SERVICE_BATTERY_THRESHOLD_CHANGED,
    HARDWARE_SERVICE_ON_BATTERY,
    HARDWARE_SERVICE_ON_USB_CHANGED,
    STEAM_SERVICE_GAME_EVENT,
)
from rcc.utils.shell import SHELL


class BoostControlHandler(FileSystemEventHandler):
    """Watcher for boost file changes"""

    def __init__(self, path: str, on_value: str, callback: Callable[[bool], None]):
        super().__init__()
        self._path = path
        self._on_value = on_value
        self._callback = callback

    def on_modified(self, event):
        if event.src_path == self._path:
            with open(self._path, "r") as f:
                content = f.read().strip()
            self._callback(content == self._on_value)


class HardwareService:
    """Hardware service"""

    CPU_PRIORITY = -15
    IO_PRIORITY = floor((CPU_PRIORITY + 20) / 5)
    IO_CLASS = 2
    BOOST_CONTROLS: List[Dict[str, str]] = [
        {
            "path": "/sys/devices/system/cpu/intel_pstate/no_turbo",
            "on": "0",
            "off": "1",
        },
        {
            "path": "/sys/devices/system/cpu/cpufreq/boost",
            "on": "1",
            "off": "0",
        },
    ]

    def __init__(self):  # pylint: disable=too-many-branches
        self._logger = Logger()
        self._logger.info("Initializing HardwareService")
        self._logger.add_tab()

        self._boost_control: Optional[Dict[str, str]] = None
        self._last_boost: Optional[bool] = None

        self._logger.info("Detecting CPU")
        self._logger.add_tab()
        self.__cpu = None
        output = SHELL.run_command("cat /proc/cpuinfo | head -n5", output=True)[1]
        if "GenuineIntel" in output:
            self.__cpu = CpuBrand.INTEL
            line = output.splitlines()[-1]
            line = line[line.index(":") + 1 :].strip()
            self._logger.info(line)
        self._logger.add_tab()

        self.__hp_cores = self.__determine_cpu_architecture()
        if self.hp_cores is not None:
            self._logger.info(f"Performance cores: {self.__hp_cores}")

        if self.__cpu == CpuBrand.INTEL:
            if PL1_SPL_CLIENT.available:
                self._logger.info("TDP control available")

        for control in self.BOOST_CONTROLS:
            if os.path.exists(control["path"]):
                self._logger.info("Boost control available")

                self._boost_control = control

                with open(self._boost_control["path"], "r") as f:
                    content = f.read().strip()
                self._last_boost = self._boost_control["on"] == content

                handler = BoostControlHandler(
                    path=self._boost_control["path"],
                    on_value=self._boost_control["on"],
                    callback=self._update_boost_status,
                )
                self._observer = Observer()
                self._observer.schedule(
                    handler,
                    path=os.path.dirname(self._boost_control["path"]),
                    recursive=False,
                )
                self._observer.start()

                break
        self._logger.rem_tab()
        self._logger.rem_tab()

        self.__gpus = []
        self.__icd_files = {}
        if SWITCHEROO_CLIENT.available:  # pylint: disable=too-many-nested-blocks
            self._logger.info("Detecting GPU")
            self._logger.add_tab()
            gpus = list(SWITCHEROO_CLIENT.gpus)
            for gpu in sorted(gpus, key=lambda x: (x["Name"])):
                self._logger.info(f"{gpu["Name"]}")
                if gpu["Discrete"]:
                    brand = GpuBrand(gpu["Name"].split(" ")[0].lower())
                    self._logger.add_tab()
                    if any(not os.path.exists(icd) for icd in self.get_icd_files(brand)):
                        self._logger.error("Missing ICD files for Vulkan")
                    else:
                        self._logger.error("ICD files found")
                        self.__gpus.append(brand)

                        if brand == GpuBrand.NVIDIA:
                            if NV_BOOST_CLIENT.available:
                                self._logger.info("Dynamic boost control available")
                            if NV_TEMP_CLIENT.available:
                                self._logger.info("Throttle temperature control available")

                    self._logger.rem_tab()

        self._logger.rem_tab()

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

    def get_icd_files(self, gpu: GpuBrand):
        """Get path to ICD files"""
        return [
            f"/usr/share/vulkan/icd.d/{gpu.value}_icd.i686.json",
            f"/usr/share/vulkan/icd.d/{gpu.value}_icd.x86_64.json",
        ]

    def _update_boost_status(self, is_on: bool):
        self._last_boost = is_on

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
            cores = sorted(set(p_cores))
            groups = []
            beg = cores[0]
            end = cores[0]

            for num in cores[1:]:
                if num == end + 1:
                    end = num
                else:
                    groups.append(f"{beg}-{end}" if beg != end else str(beg))
                    beg = end = num

            groups.append(f"{beg}-{end}" if beg != end else str(beg))
            return ",".join(groups)

        return None

    @property
    def icd_files(self) -> list[str] | None:
        """ICD files for GPU"""
        return self.__icd_files

    @property
    def gpus(self) -> list[GpuBrand]:
        """GPU brand"""
        return self.__gpus

    @property
    def cpu(self) -> CpuBrand | None:
        """CPU brand"""
        return self.__cpu

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
    def boost_allowed(self) -> None:
        """Boost flag"""
        return self._boost_control is not None

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

    def set_boost_status(self, enabled: bool):
        """Enable/disable cpu boost"""
        if self.boost_allowed:
            self._logger.info(f"CPU boost: {"ENABLED" if enabled else "DISABLED"}")
            target = "on" if enabled else "off"
            value = self._boost_control[target]
            path = self._boost_control["path"]

            SHELL.run_command(f"echo '{value}' | tee {path}", True)

    def __monitor_for_usb(self) -> None:  # pylint: disable=too-many-locals, too-many-branches
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

        for action, _ in monitor:  # pylint: disable=too-many-nested-blocks
            if action in ["add", "remove"]:
                self._usb_mutex.acquire(True)  # pylint: disable=consider-using-with
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

    def apply_proccess_optimizations(self, pid: int, first_run=True):
        """Apply optimizations to the process tree"""
        if first_run:
            self._logger.info(
                f"Setting CPU affinity to cores {self.__hp_cores}, "
                + f"priority to {self.CPU_PRIORITY} and "
                + f"IO priority to {self.IO_PRIORITY}"
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
                            # + f"ionice -c {self.IO_CLASS} -n {self.IO_PRIORITY} -p {pid} && "
                            # + f"taskset -cp {self.__hp_cores} {pid}",
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
            # secs = 60
            # Thread(target=lambda: self.__keep_optimizing(pid, secs, processed)).start()
            # self._logger.info(f"Optimizations will keep running in background for {secs} seconds")

        return len(processed)

    """
    def __keep_optimizing(self, pid, time_len, processed):
        t0 = time.time()
        beg = len(processed)
        last = beg
        while time.time() - t0 < time_len:
            time.sleep(0.5)
            last = max(last, self.apply_proccess_optimizations(pid, False))

        if last - beg > 0:
            self._logger.info(f"Optimization finished for {last-beg} additional processes")
    """

    def set_cpu_tdp(self, profile: PerformanceProfile):
        """Set CPU TDP configuration"""
        if HARDWARE_SERVICE.cpu == CpuBrand.INTEL:
            pl1 = profile.ac_intel_pl1_spl
            pl2 = profile.ac_intel_pl2_sppt
            if pl1 is not None:
                time.sleep(0.05)
                self._logger.info(f"{self.cpu.capitalize()} CPU")

                if UPOWER_CLIENT.on_battery:
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
                if UPOWER_CLIENT.on_battery:
                    nv = profile.battery_nv_boost
                    nt = profile.battery_nv_temp

                if nv is not None:
                    self._logger.info(f"  Dynamic Boost: {nv}W")
                    NV_BOOST_CLIENT.current_value = nv

                if nt is not None:
                    self._logger.info(f"  Throttle temp: {nt}ºC")
                    NV_TEMP_CLIENT.current_value = nt


HARDWARE_SERVICE = HardwareService()
