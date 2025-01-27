from threading import Lock
from typing import Callable, Dict, List, Optional

import os
import subprocess

import concurrent
from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

from rcc.communications.client.dbus.fan_curves_client import fan_curves_client
from rcc.communications.client.dbus.platform_client import platform_client
from rcc.communications.client.dbus.power_profiles_client import power_profile_client
from rcc.communications.client.dbus.upower_client import upower_client
from rcc.gui.notifier import notifier
from rcc.models.battery_threshold import BatteryThreshold
from rcc.models.performance_profile import PerformanceProfile
from rcc.models.power_profile import PowerProfile
from rcc.utils.configuration import configuration
from rcc.utils.beans import translator
from rcc.utils.beans import cryptography
from rcc.utils.beans import event_bus
from framework.logger import Logger, logged_method
from framework.singleton import singleton


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
class PlatformService:
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
        self._logger.info("Initializing PlatformService")
        self._logger.add_tab()
        self._lock = Lock()
        self._ac_events_enabled = True

        self._boost_control: Optional[Dict[str, str]] = None
        self._last_boost: Optional[bool] = None
        self._observer = Observer()

        self._performance_profile = PerformanceProfile(configuration.platform.profiles.profile)
        self._platform_profile = platform_client.platform_profile
        self._battery_charge_limit = platform_client.charge_control_end_threshold
        self.on_bat = upower_client.on_battery

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

        platform_client.change_platform_profile_on_ac = False
        platform_client.change_platform_profile_on_battery = False

        upower_client.on_property_change("OnBattery", self._on_ac_battery_change)
        event_bus.on("GamesService.gameEvent", self._on_game_event)

        self._logger.rem_tab()

    @property
    def performance_profile(self) -> PerformanceProfile:
        """Performance profile"""
        return self._performance_profile

    def _update_boost_status(self, is_on: bool):
        self._last_boost = is_on

    @property
    def battery_charge_limit(self) -> BatteryThreshold:
        """Get current battery charge limit"""
        return self._battery_charge_limit

    def _on_game_event(self, count):
        self._ac_events_enabled = count == 0

    def _set_power_profile(self, value):
        self._platform_profile = PowerProfile(value)

    def _on_ac_battery_change(self, on_battery: bool, muted=False, force=False):
        if self._ac_events_enabled or force:
            self.on_bat = on_battery
            policy = PerformanceProfile(configuration.platform.profiles.profile)
            if on_battery:
                policy = PerformanceProfile.QUIET

            if not muted:
                self._logger.info(
                    f"AC cord {"dis" if on_battery else ""}connected, battery {"dis" if not on_battery else ""}engaged"
                )
                self._logger.add_tab()
            self.set_performance_profile(policy, True)
            if not muted:
                self._logger.rem_tab()

    @logged_method
    def set_performance_profile(  # pylint: disable=R0914
        self, profile: PerformanceProfile, temporal=False, game_name: str = None, force=False
    ) -> None:
        """Establish performance profile"""
        with self._lock:
            profile_name = profile.name
            if self._performance_profile != profile or force:
                platform_profile = profile.get_platform_profile()
                power_profile = profile.get_power_profile()
                boost_enabled = profile.get_boost_enabled()
                try:
                    self._logger.info(
                        f"Setting profile '{profile_name.lower()}' {f"for game {game_name}" if game_name is not None else ""}"  # pylint: disable=C0301
                    )
                    self._logger.add_tab()

                    def set_throttle_policy():
                        self._logger.debug(f"Throttle policy: {platform_profile.name}")
                        platform_client.platform_profile = platform_profile
                        self._platform_profile = platform_profile

                    def set_fan_curves():
                        self._logger.debug(f"Fan curve: {platform_profile.name}")
                        fan_curves_client.set_curves_to_defaults(platform_profile)
                        fan_curves_client.reset_profile_curves(platform_profile)
                        fan_curves_client.set_fan_curves_enabled(platform_profile, True)

                    def set_power_profile():
                        self._logger.debug(f"Power profile: {power_profile.name}")
                        power_profile_client.active_profile = power_profile

                    def handle_boost():
                        no_boost_reason = None
                        if self._boost_control is None:
                            no_boost_reason = "unsupported CPU"

                        if no_boost_reason is not None:
                            self._logger.debug(f"Boost: omitted due to {no_boost_reason}")
                        else:
                            if boost_enabled:
                                self._logger.debug("Boost: ENABLED")
                            else:
                                self._logger.debug("Boost: DISABLED")

                            target = "on" if boost_enabled else "off"
                            value = self._boost_control[target]
                            path = self._boost_control["path"]

                            command = f'echo "{cryptography.decrypt_string(configuration.settings.password)}" | sudo -S bash -c "echo \'{value}\' | tee {path}" &>> /dev/null'  # pylint: disable=C0301
                            subprocess.run(command, shell=True, check=True)

                    # Ejecutar las operaciones de forma concurrente
                    with concurrent.futures.ThreadPoolExecutor() as executor:
                        futures = [
                            executor.submit(handle_boost),
                            executor.submit(set_throttle_policy),
                            executor.submit(set_fan_curves),
                            executor.submit(set_power_profile),
                        ]
                        # Esperar a que todas las operaciones terminen
                        concurrent.futures.wait(futures)

                    self._logger.rem_tab()
                    self._logger.info("Profile setted succesfully")

                    if not temporal and not self.on_bat:
                        configuration.platform.profiles.profile = profile.value
                        configuration.save_config()

                    if game_name is None:
                        notifier.show_toast(
                            translator.translate(
                                "profile.applied",
                                {"profile": translator.translate(f"label.profile.{profile_name}").lower()},
                            )
                        )
                    else:
                        notifier.show_toast(
                            translator.translate(
                                "profile.applied.for.game",
                                {
                                    "profile": translator.translate(f"label.profile.{profile_name}").lower(),
                                    "game": game_name,
                                },
                            )
                        )
                    self._performance_profile = profile
                    event_bus.emit(
                        "PlatformService.performance_profile",
                        self._performance_profile,
                    )
                except Exception as error:
                    self._logger.error(f"Couldn't set profile: {error}")
                    self._logger.rem_tab()
            else:
                self._logger.info(f"Profile {profile_name.lower()} already setted")
                if game_name is None:
                    notifier.show_toast(
                        translator.translate(
                            "profile.applied",
                            {"profile": translator.translate(f"label.profile.{profile_name}").lower()},
                        )
                    )

    def set_battery_threshold(self, value: BatteryThreshold) -> None:
        """Set battery charge threshold"""
        if value != self._battery_charge_limit:
            platform_client.charge_control_end_threshold = value
            self._battery_charge_limit = value
            event_bus.emit("PlatformService.battery_threshold", value)
            notifier.show_toast(translator.translate("applied.battery.threshold", {"value": value.value}))

    def restore_profile(self):
        """Restore persited profile"""
        if upower_client.on_battery:
            self._logger.info("Laptop running on battery")
            self._logger.add_tab()
            self._on_ac_battery_change(upower_client.on_battery, True, True)
            self._logger.rem_tab()
        else:
            self._logger.info("Laptop running on AC")
            self._logger.add_tab()
            self.set_performance_profile(PerformanceProfile(configuration.platform.profiles.profile), True, None, False)
            self._logger.rem_tab()


platform_service = PlatformService()
