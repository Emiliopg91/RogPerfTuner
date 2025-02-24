import concurrent
import os
import time
from threading import Lock
from typing import Callable, Dict, List, Optional

from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

from framework.logger import Logger, logged_method
from framework.singleton import singleton
from rcc.communications.client.dbus.asus.armoury.intel.pl1_spl_client import PL1_SPL_CLIENT
from rcc.communications.client.dbus.asus.armoury.intel.pl2_sppt_client import PL2_SPPT_CLIENT
from rcc.communications.client.dbus.asus.armoury.nvidia.nv_boost_client import NV_BOOST_CLIENT
from rcc.communications.client.dbus.asus.armoury.nvidia.nv_temp_client import NV_TEMP_CLIENT
from rcc.communications.client.dbus.asus.core.fan_curves_client import FAN_CURVES_CLIENT
from rcc.communications.client.dbus.asus.core.platform_client import PLATFORM_CLIENT
from rcc.communications.client.dbus.linux.power_profiles_client import POWER_PROFILE_CLIENT
from rcc.communications.client.dbus.linux.upower_client import UPOWER_CLIENT
from rcc.gui.notifier import NOTIFIER
from rcc.models.cpu_brand import CpuBrand
from rcc.models.cpu_governor import CpuGovernor
from rcc.models.gpu_brand import GpuBrand
from rcc.models.performance_profile import PerformanceProfile
from rcc.models.platform_profile import PlatformProfile
from rcc.models.power_profile import PowerProfile
from rcc.services.hardware_service import HARDWARE_SERVICE
from rcc.utils.beans import EVENT_BUS, TRANSLATOR
from rcc.utils.configuration import CONFIGURATION
from rcc.utils.events import HARDWARE_SERVICE_ON_BATTERY, PLATFORM_SERVICE_PROFILE_CHANGED, STEAM_SERVICE_GAME_EVENT
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


@singleton
class ProfileService:
    """Service for platform setting"""

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

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing ProfileService")
        self._logger.add_tab()
        self._lock = Lock()
        self._ac_events_enabled = True

        self._boost_control: Optional[Dict[str, str]] = None
        self._last_boost: Optional[bool] = None
        self._observer = Observer()

        self._performance_profile = PerformanceProfile(CONFIGURATION.platform.profiles.profile)
        self._platform_profile = PLATFORM_CLIENT.platform_profile
        self._battery_charge_limit = PLATFORM_CLIENT.charge_control_end_threshold
        self.on_bat = UPOWER_CLIENT.on_battery

        for control in self.BOOST_CONTROLS:
            if os.path.exists(control["path"]):
                self._boost_control = control
                break

        if self._boost_control:
            with open(self._boost_control["path"], "r") as f:
                content = f.read().strip()
            self._last_boost = self._boost_control["on"] == content

            handler = BoostControlHandler(
                path=self._boost_control["path"],
                on_value=self._boost_control["on"],
                callback=self._update_boost_status,
            )
            self._observer.schedule(
                handler,
                path=os.path.dirname(self._boost_control["path"]),
                recursive=False,
            )
            self._observer.start()

        PLATFORM_CLIENT.change_platform_profile_on_ac = False
        PLATFORM_CLIENT.change_platform_profile_on_battery = False

        EVENT_BUS.on(HARDWARE_SERVICE_ON_BATTERY, self._on_ac_battery_change)
        EVENT_BUS.on(STEAM_SERVICE_GAME_EVENT, self._on_game_event)

        self._logger.rem_tab()

    @property
    def performance_profile(self) -> PerformanceProfile:
        """Performance profile"""
        return self._performance_profile

    def _update_boost_status(self, is_on: bool):
        self._last_boost = is_on

    def _on_game_event(self, count):
        self._ac_events_enabled = count == 0

    def _set_power_profile(self, value):
        self._platform_profile = PowerProfile(value)

    def _on_ac_battery_change(self, on_battery: bool, force=False):
        if self._ac_events_enabled or force:
            self.on_bat = on_battery
            policy = PerformanceProfile(CONFIGURATION.platform.profiles.profile)
            if on_battery:
                policy = PerformanceProfile.QUIET

            self.set_performance_profile(policy, True, force=True)

    @logged_method
    def set_performance_profile(  # pylint: disable=R0914
        self, profile: PerformanceProfile, temporal=False, game_name: str = None, force=False
    ) -> None:
        """Establish performance profile"""
        with self._lock:
            profile_name = profile.name
            if self._performance_profile != profile or force:
                platform_profile = profile.platform_profile
                power_profile = profile.power_profile
                cpu_governor = profile.battery_governor if self.on_bat else profile.ac_governor
                boost_enabled = profile.battery_boost if self.on_bat else profile.ac_boost
                try:
                    self._logger.info(
                        f"Setting {profile_name} profile {f"for game {game_name}" if game_name is not None else ""}"  # pylint: disable=C0301
                    )
                    self._logger.add_tab()

                    # Ejecutar las operaciones de forma concurrente
                    with concurrent.futures.ThreadPoolExecutor() as executor:
                        futures = [
                            executor.submit(lambda: self.__set_boost(boost_enabled)),
                            executor.submit(lambda: self.__set_cpu_governor(cpu_governor)),
                            executor.submit(lambda: self.__set_power_profile(power_profile)),
                            executor.submit(lambda: self.__set_fan_curves(platform_profile)),
                            executor.submit(lambda: self.__set_throttle_policy(profile, platform_profile)),
                        ]
                        # Esperar a que todas las operaciones terminen
                        concurrent.futures.wait(futures)

                    self._logger.rem_tab()
                    self._logger.info("Profile setted succesfully")

                    if not temporal and not self.on_bat:
                        CONFIGURATION.platform.profiles.profile = profile.value
                        CONFIGURATION.save_config()

                    if game_name is None:
                        NOTIFIER.show_toast(
                            TRANSLATOR.translate(
                                "profile.applied",
                                {"profile": TRANSLATOR.translate(f"label.profile.{profile_name}").lower()},
                            )
                        )
                    else:
                        NOTIFIER.show_toast(
                            TRANSLATOR.translate(
                                "profile.applied.for.game",
                                {
                                    "profile": TRANSLATOR.translate(f"label.profile.{profile_name}").lower(),
                                    "game": game_name,
                                },
                            )
                        )
                    self._performance_profile = profile
                    EVENT_BUS.emit(
                        PLATFORM_SERVICE_PROFILE_CHANGED,
                        self._performance_profile,
                    )
                except Exception as error:
                    self._logger.error(f"Couldn't set profile: {error}")
                    self._logger.rem_tab()
            else:
                self._logger.info(f"Profile {profile_name.lower()} already setted")
                if game_name is None:
                    NOTIFIER.show_toast(
                        TRANSLATOR.translate(
                            "profile.applied",
                            {"profile": TRANSLATOR.translate(f"label.profile.{profile_name}").lower()},
                        )
                    )

    def restore_profile(self):
        """Restore persited profile"""
        if self.on_bat:
            self._logger.info("Laptop running on battery")
            self._logger.add_tab()
            self._on_ac_battery_change(True, True)
            self._logger.rem_tab()
        else:
            self._logger.info("Laptop running on AC")
            self._logger.add_tab()
            self.set_performance_profile(PerformanceProfile(CONFIGURATION.platform.profiles.profile), True, None, True)
            self._logger.rem_tab()

    def __set_throttle_policy(self, profile: PerformanceProfile, platform_profile: PlatformProfile):
        try:
            platform_profile = profile.platform_profile
            self._logger.info(f"Throttle policy: {platform_profile.name}")
            self._logger.add_tab()
            PLATFORM_CLIENT.platform_profile = platform_profile
            self._platform_profile = platform_profile

            PLATFORM_CLIENT.enable_ppt_group = True
            time.sleep(0.05)

            if HARDWARE_SERVICE.cpu == CpuBrand.INTEL:
                pl1 = profile.ac_pl1_spl
                pl2 = profile.ac_pl2_sppt
                if pl1 is not None:
                    time.sleep(0.05)
                    self._logger.info("CPU power")

                    if self.on_bat:
                        pl1 = profile.battery_pl1_spl
                        pl2 = profile.battery_pl2_sppt

                    self._logger.info(f"  PL1: {pl1}W")
                    PL1_SPL_CLIENT.current_value = PL1_SPL_CLIENT.default_value
                    time.sleep(0.05)
                    PL1_SPL_CLIENT.current_value = pl1

                    if pl2 is not None:
                        self._logger.info(f"  PL2: {pl2}W")
                        PL2_SPPT_CLIENT.current_value = PL2_SPPT_CLIENT.default_value
                        time.sleep(0.05)
                        PL2_SPPT_CLIENT.current_value = pl2

            if HARDWARE_SERVICE.gpu == GpuBrand.NVIDIA:
                nv = profile.ac_nv_boost
                nt = profile.ac_nv_temp
                if nv is not None or nt is not None:
                    self._logger.info("GPU power")
                    if self.on_bat:
                        nv = profile.battery_nv_boost
                        nt = profile.battery_nv_temp

                    if nv is not None:
                        self._logger.info(f"  BST: {nv}W")
                        NV_BOOST_CLIENT.current_value = nv

                    if nt is not None:
                        self._logger.info(f"  TEM: {nt}ºC")
                        NV_TEMP_CLIENT.current_value = nt

            self._logger.rem_tab()
        except Exception as e:
            self._logger.error(f"Error while setting platform policy: {e}")

    def __set_fan_curves(self, platform_profile: PlatformProfile):
        try:
            self._logger.info(f"Fan curve: {platform_profile.name}")
            FAN_CURVES_CLIENT.set_curves_to_defaults(platform_profile)
            FAN_CURVES_CLIENT.reset_profile_curves(platform_profile)
            FAN_CURVES_CLIENT.set_fan_curves_enabled(platform_profile, True)
        except Exception as e:
            self._logger.error(f"Error while setting fan curve: {e}")

    def __set_power_profile(self, power_profile: PowerProfile):
        try:
            self._logger.info(f"Power profile: {power_profile.name}")
            POWER_PROFILE_CLIENT.active_profile = power_profile
        except Exception as e:
            self._logger.error(f"Error while setting power profile: {e}")

    def __set_cpu_governor(self, governor: CpuGovernor):
        try:
            self._logger.info(f"CPU governor: {governor.name}")
            SHELL.run_command(f"cpupower frequency-set -g {governor.value}", True)
        except Exception as e:
            self._logger.error(f"Error while setting CPU governor: {e}")

    def __set_boost(self, boost_enabled: bool):
        try:
            if self._boost_control is not None:
                if boost_enabled:
                    self._logger.info("Boost: ENABLED")
                else:
                    self._logger.info("Boost: DISABLED")

                target = "on" if boost_enabled else "off"
                value = self._boost_control[target]
                path = self._boost_control["path"]

                SHELL.run_command(f"echo '{value}' | tee {path}", True)
        except Exception as e:
            self._logger.error(f"Error while setting boost mode: {e}")


PROFILE_SERVICE = ProfileService()
