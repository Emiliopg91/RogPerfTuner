from threading import Lock
from typing import Callable, Dict, List, Optional

import os
import subprocess

import concurrent
from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

from rcc.clients.dbus.fan_curves_client import fan_curves_client
from rcc.clients.dbus.platform_client import platform_client
from rcc.clients.dbus.power_profiles_client import power_profile_client
from rcc.clients.dbus.upower_client import upower_client
from rcc.gui.notifier import notifier
from rcc.models.battery_threshold import BatteryThreshold
from rcc.models.boost import Boost
from rcc.models.power_profile import PowerProfile
from rcc.models.platform_profile import PlatformProfile
from rcc.utils.configuration import configuration
from rcc.utils.cryptography import cryptography
from rcc.utils.event_bus import event_bus
from rcc.utils.logger import Logger, logged_method
from rcc.utils.singleton import singleton
from rcc.utils.translator import translator


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

    THROTTLE_POWER_ASSOC: dict[PlatformProfile, PowerProfile] = {
        PlatformProfile.QUIET: PowerProfile.POWER_SAVER,
        PlatformProfile.BALANCED: PowerProfile.BALANCED,
        PlatformProfile.PERFORMANCE: PowerProfile.PERFORMANCE,
    }

    THROTTLE_BOOST_ASSOC: dict[PlatformProfile, bool] = {
        PlatformProfile.QUIET: False,
        PlatformProfile.BALANCED: False,
        PlatformProfile.PERFORMANCE: True,
    }

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

        self._thermal_throttle_profile = platform_client.platform_profile
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
            self._boost_mode = Boost(configuration.platform.profiles.boost)
        else:
            self._boost_mode = Boost.OFF
            configuration.platform.profiles.boost = Boost.OFF
            configuration.save_config()

        platform_client.change_platform_profile_on_ac = False
        platform_client.change_platform_profile_on_battery = False

        upower_client.on_property_change("OnBattery", self._on_ac_battery_change)
        event_bus.on("GamesService.gameEvent", self._on_game_event)

        self._logger.rem_tab()

    @property
    def thermal_throttle_profile(self):
        """Getter for thermal throttle profile"""
        return self._thermal_throttle_profile

    def _update_boost_status(self, is_on: bool):
        self._last_boost = is_on

    def get_thermal_throttle_profile(self) -> PlatformProfile:
        """Get current profile"""
        return self._thermal_throttle_profile

    @property
    def boost_mode(self) -> Boost:
        """Get current boost mode"""
        return self._boost_mode

    @property
    def battery_charge_limit(self) -> BatteryThreshold:
        """Get current battery charge limit"""
        return self._battery_charge_limit

    def _on_game_event(self, count):
        self._ac_events_enabled = count == 0

    def _set_power_profile(self, value):
        self._thermal_throttle_profile = PowerProfile(value)

    def _on_ac_battery_change(self, on_battery: bool, muted=False, force=False):
        if self._ac_events_enabled or force:
            self.on_bat = on_battery
            policy = PlatformProfile(configuration.platform.profiles.profile)
            if on_battery:
                policy = PlatformProfile.QUIET

            if not muted:
                self._logger.info(
                    f"AC cord {"dis" if on_battery else ""}connected, battery {"dis" if not on_battery else ""}engaged"
                )
                self._logger.add_tab()
            self.set_thermal_throttle_policy(policy, True)
            if not muted:
                self._logger.rem_tab()

    @logged_method
    def set_thermal_throttle_policy(
        self, policy: PlatformProfile, temporal=False, game_name: str = None, force=False
    ) -> None:
        """Establish thermal throttle policy"""
        with self._lock:
            policy_name = policy.name
            if self._thermal_throttle_profile != policy or force:
                power_profile = self.THROTTLE_POWER_ASSOC[policy]

                try:
                    self._logger.info(
                        f"Setting profile '{policy_name.lower()}' {f"for game {game_name}" if game_name is not None else ""}"
                    )
                    self._logger.add_tab()

                    def set_throttle_policy():
                        self._logger.debug(f"Throttle policy: {policy_name}")
                        platform_client.platform_profile = policy
                        self._thermal_throttle_profile = policy

                    def set_fan_curves():
                        self._logger.debug(f"Fan curve: {policy_name}")
                        fan_curves_client.set_curves_to_defaults(policy)
                        fan_curves_client.reset_profile_curves(policy)
                        fan_curves_client.set_fan_curves_enabled(policy, True)

                    def set_power_profile():
                        self._logger.debug(f"Power profile: {power_profile.name}")
                        power_profile_client.active_profile = power_profile

                    def handle_boost():
                        no_boost_reason = None
                        if self._boost_control is None:
                            no_boost_reason = "unsupported CPU"
                        elif self._boost_mode != Boost.AUTO:
                            no_boost_reason = f"{self._boost_mode.name} mode"

                        if no_boost_reason is not None:
                            self._logger.debug(f"Boost: omitted due to {no_boost_reason}")
                        elif self._boost_mode == Boost.AUTO:
                            enabled = self.THROTTLE_BOOST_ASSOC[policy]
                            if enabled:
                                self._logger.debug("Boost: ENABLED")
                            else:
                                self._logger.debug("Boost: DISABLED")
                            self._apply_boost(enabled)

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
                        configuration.platform.profiles.profile = policy.value
                        configuration.save_config()

                    if game_name is None:
                        notifier.show_toast(
                            translator.translate(
                                "profile.applied",
                                {"profile": translator.translate(f"label.profile.{policy_name}").lower()},
                            )
                        )
                    else:
                        notifier.show_toast(
                            translator.translate(
                                "profile.applied.for.game",
                                {
                                    "profile": translator.translate(f"label.profile.{policy_name}").lower(),
                                    "game": game_name,
                                },
                            )
                        )
                    event_bus.emit(
                        "PlatformService.platform_profile",
                        self._thermal_throttle_profile,
                    )
                except Exception as error:
                    self._logger.error(f"Couldn't set profile: {error}")
                    self._logger.rem_tab()
            else:
                self._logger.info(f"Profile {policy_name.lower()} already setted")

    def set_battery_threshold(self, value: BatteryThreshold) -> None:
        """Set battery charge threshold"""
        if value != self._battery_charge_limit:
            platform_client.charge_control_end_threshold = value
            self._battery_charge_limit = value
            event_bus.emit("PlatformService.battery_threshold", value)
            notifier.show_toast(translator.translate("applied.battery.threshold", {"value": value.value}))

    def _apply_boost(self, enabled: bool):
        if self._boost_control is not None and enabled != self._last_boost:
            target = "on" if enabled else "off"
            value = self._boost_control[target]
            path = self._boost_control["path"]

            command = f'echo "{cryptography.decrypt_string(configuration.settings.password)}" | sudo -S bash -c "echo \'{value}\' | tee {path}" &>> /dev/null'  # pylint: disable=C0301
            subprocess.run(command, shell=True, check=True)

    def set_boost_mode(self, boost: Boost) -> None:
        """Set boost mode"""
        if boost != self._boost_mode:
            self._logger.info(f"Setting boost mode to {boost.name}")
            self._logger.add_tab()

            enabled = self.THROTTLE_BOOST_ASSOC[self._thermal_throttle_profile]
            if boost == Boost.OFF:
                enabled = False

            if enabled != self._last_boost:
                self._apply_boost(enabled)
            self._logger.rem_tab()

            self._boost_mode = boost
            configuration.platform.profiles.boost = boost.value
            configuration.save_config()
            self._logger.info("Boost mode setted succesfully")

            event_bus.emit("PlatformService.boost", self._boost_mode)

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
            self.set_thermal_throttle_policy(
                PlatformProfile(configuration.platform.profiles.profile), True, None, False
            )
            self._logger.rem_tab()


platform_service = PlatformService()
