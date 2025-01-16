from threading import Lock
from typing import Callable, Dict, List, Optional

import os
import subprocess
import threading

from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

from lib.clients.dbus.fan_curves_client import fan_curves_client
from lib.clients.dbus.platform_client import platform_client
from lib.clients.dbus.power_profiles_client import power_profile_client
from lib.clients.dbus.upower_client import upower_client
from lib.gui.notifier import notifier
from lib.models.battery_threshold import BatteryThreshold
from lib.models.boost import Boost
from lib.models.power_profile import PowerProfile
from lib.models.thermal_throttle_profile import ThermalThrottleProfile
from lib.utils.configuration import configuration
from lib.utils.cryptography import cryptography
from lib.utils.event_bus import event_bus
from lib.utils.logger import Logger
from lib.utils.singleton import singleton
from lib.utils.translator import translator


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

    THROTTLE_POWER_ASSOC: dict[ThermalThrottleProfile, PowerProfile] = {
        ThermalThrottleProfile.QUIET: PowerProfile.POWER_SAVER,
        ThermalThrottleProfile.BALANCED: PowerProfile.BALANCED,
        ThermalThrottleProfile.PERFORMANCE: PowerProfile.PERFORMANCE,
    }

    THROTTLE_BOOST_ASSOC: dict[ThermalThrottleProfile, bool] = {
        ThermalThrottleProfile.QUIET: False,
        ThermalThrottleProfile.BALANCED: False,
        ThermalThrottleProfile.PERFORMANCE: True,
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
        self._lock = Lock()

        self._boost_control: Optional[Dict[str, str]] = None
        self._last_boost: Optional[bool] = None
        self._observer = Observer()

        self._thermal_throttle_profile = platform_client.throttle_thermal_policy
        self._battery_charge_limit = platform_client.charge_control_end_threshold

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

        self.restore_profile()

        platform_client.on_property_change("ThrottleThermalPolicy", self._set_thermal_throttle_profile_async)
        platform_client.on_property_change("ChargeControlEndThreshold", self._set_battery_threshold)
        upower_client.on_property_change("OnBattery", self._on_ac_battery_change)

    @property
    def thermal_throttle_profile(self):
        """Getter for thermal throttle profile"""
        return self._thermal_throttle_profile

    def _update_boost_status(self, is_on: bool):
        self._last_boost = is_on

    def get_thermal_throttle_profile(self) -> ThermalThrottleProfile:
        """Get current profile"""
        return self._thermal_throttle_profile

    def get_boost_mode(self) -> Boost:
        """Get current boost mode"""
        return self._boost_mode

    def get_battery_charge_limit(self) -> BatteryThreshold:
        """Get current battery charge limit"""
        return self._battery_charge_limit

    def _set_battery_threshold(self, value):
        self._battery_charge_limit = BatteryThreshold(int.from_bytes(value))
        event_bus.emit("PlatformService.battery_threshold", self._battery_charge_limit)

    def _set_thermal_throttle_profile_async(self, value):
        threading.Thread(
            name="PlatformService",
            target=self._set_thermal_throttle_profile,
            args=[value],
        ).start()

    def _set_thermal_throttle_profile(self, value):
        with self._lock:
            self._thermal_throttle_profile = ThermalThrottleProfile(value)
            event_bus.emit(
                "PlatformService.thermal_throttle_profile",
                self._thermal_throttle_profile,
            )

    def _set_power_profile(self, value):
        self._thermal_throttle_profile = PowerProfile(value)

    def _on_ac_battery_change(self, on_battery):
        """
        policy = ThermalThrottleProfile.PERFORMANCE
        if on_battery:
            policy = ThermalThrottleProfile.QUIET

        self.set_thermal_throttle_policy(policy, True)
        """

    def set_thermal_throttle_policy(
        self, policy: ThermalThrottleProfile, temporal=False, game_name: str = None, force=False
    ) -> None:
        """Establish thermal throttle policy"""
        with self._lock:
            policy_name = policy.name
            power_profile = self.THROTTLE_POWER_ASSOC[policy]

            if self._thermal_throttle_profile != policy or force:
                try:
                    no_boost_reason = None
                    if self._boost_control is None:
                        no_boost_reason = "unsupported"

                    self._logger.info(f"Setting {policy_name.lower()} profile")
                    self._logger.add_tab()

                    self._logger.info(f"Throttle policy: {policy_name}")
                    platform_client.throttle_thermal_policy = policy
                    self._thermal_throttle_profile = policy

                    self._logger.info(f"Fan curve: {policy_name}")
                    fan_curves_client.set_curves_to_defaults(policy)
                    fan_curves_client.reset_profile_curves(policy)
                    fan_curves_client.set_fan_curves_enabled(policy, True)

                    self._logger.info(f"Power profile: {power_profile.name}")
                    power_profile_client.active_profile = power_profile

                    if no_boost_reason is not None:
                        self._logger.info(f"Boost: omitted due to {no_boost_reason}")
                    elif self._boost_mode == Boost.AUTO:
                        enabled = self.THROTTLE_BOOST_ASSOC[policy]
                        if enabled:
                            self._logger.info("Boost: ENABLED")
                        else:
                            self._logger.info("Boost: DISABLED")
                        self._apply_boost(enabled)
                    self._logger.rem_tab()
                    self._logger.info("Profile setted succesfully")

                    if not temporal:
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
                except Exception as error:
                    self._logger.error(f"Couldn't set profile: {error}")
                    self._logger.rem_tab()

    def set_battery_threshold(self, value: BatteryThreshold) -> None:
        """Set battery charge threshold"""
        if value != self._battery_charge_limit:
            platform_client.charge_control_end_threshold = value
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
        self.set_thermal_throttle_policy(
            ThermalThrottleProfile(configuration.platform.profiles.profile), True, None, True
        )


platform_service = PlatformService()
