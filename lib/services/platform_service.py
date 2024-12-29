from ..clients.dbus.fan_curves_client import fan_curves_client
from ..clients.dbus.platform_client import platform_client
from ..clients.dbus.power_profiles_client import power_profile_client
from ..clients.dbus.upower_client import upower_client
from ..gui.notifier import notifier
from ..models.battery_threshold import BatteryThreshold
from ..models.power_profile import PowerProfile
from ..models.thermal_throttle_profile import ThermalThrottleProfile
from ..utils.configuration import configuration
from ..utils.cryptography import cryptography
from ..utils.event_bus import event_bus
from ..utils.logger import Logger
from ..utils.singleton import singleton
from ..utils.translator import translator

from threading import Lock
from typing import Callable, Dict, List, Optional
from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

import os
import subprocess
import threading


class BoostControlHandler(FileSystemEventHandler):
    def __init__(self, path: str, on_value: str, callback: Callable[[bool], None]):
        super().__init__()
        self.path = path
        self.on_value = on_value
        self.callback = callback

    def on_modified(self, event):
        if event.src_path == self.path:
            with open(self.path, "r") as f:
                content = f.read().strip()
            self.callback(content == self.on_value)


@singleton
class PlatformService:
    def __init__(self):
        self.logger = Logger("PlatformService")
        self.lock = Lock()

        self.boost_control: Optional[Dict[str, str]] = None
        self.last_boost: Optional[bool] = None
        self.observer = Observer()

        self.thermal_throttle_profile = platform_client.throttleThermalPolicy
        platform_client.on(
            "ThrottleThermalPolicy", self._set_thermal_throttle_profile_async
        )

        self.battery_charge_limit = platform_client.chargeControlEndThreshold
        platform_client.on("ChargeControlEndThreshold", self._set_battery_threshold)

        upower_client.on("OnBattery", self._on_ac_battery_change)

        self.throttle_power_assoc: dict[ThermalThrottleProfile, PowerProfile] = {
            ThermalThrottleProfile.QUIET: PowerProfile.POWER_SAVER,
            ThermalThrottleProfile.BALANCED: PowerProfile.BALANCED,
            ThermalThrottleProfile.PERFORMANCE: PowerProfile.PERFORMANCE,
        }

        self.throttle_boost_assoc: dict[ThermalThrottleProfile, bool] = {
            ThermalThrottleProfile.QUIET: False,
            ThermalThrottleProfile.BALANCED: False,
            ThermalThrottleProfile.PERFORMANCE: True,
        }

        self.every_boost_control: List[Dict[str, str]] = [
            {
                "path": "/sys/devices/system/cpu/intel_pstate/no_turbo",
                "on": "0",
                "off": "1",
            },
            {"path": "/sys/devices/system/cpu/cpufreq/boost", "on": "1", "off": "0"},
        ]

        for control in self.every_boost_control:
            if os.path.exists(control["path"]):
                self.boost_control = control
                break

        if self.boost_control:
            with open(self.boost_control["path"], "r") as f:
                content = f.read().strip()
            self.last_boost = self.boost_control["on"] == content

            handler = BoostControlHandler(
                path=self.boost_control["path"],
                on_value=self.boost_control["on"],
                callback=self._update_boost_status,
            )
            self.observer.schedule(
                handler,
                path=os.path.dirname(self.boost_control["path"]),
                recursive=False,
            )
            self.observer.start()

        self.set_thermal_throttle_policy(
            ThermalThrottleProfile(configuration.config.platform.profiles.last),
            False,
            True,
            True,
        )

    def _update_boost_status(self, is_on: bool):
        self.last_boost = is_on

    def get_thermal_throttle_profile(self) -> ThermalThrottleProfile:
        return self.thermal_throttle_profile

    def get_battery_charge_limit(self) -> BatteryThreshold:
        return self.battery_charge_limit

    def _set_battery_threshold(self, value):
        self.battery_charge_limit = BatteryThreshold(int.from_bytes(value))
        event_bus.emit("PlatformService.battery_threshold", self.battery_charge_limit)

    def _set_thermal_throttle_profile_async(self, value):
        threading.Thread(
            target=self._set_thermal_throttle_profile, args=[value]
        ).start()

    def _set_thermal_throttle_profile(self, value):
        with self.lock:
            self.thermal_throttle_profile = ThermalThrottleProfile(value)
            event_bus.emit(
                "PlatformService.thermal_throttle_profile",
                self.thermal_throttle_profile,
            )

    def _set_power_profile(self, value):
        self.thermal_throttle_profile = PowerProfile(value)

    def _on_ac_battery_change(self, on_battery):
        policy = ThermalThrottleProfile.PERFORMANCE
        if on_battery:
            policy = ThermalThrottleProfile.QUIET

        self.set_thermal_throttle_policy(policy, True)

    def set_thermal_throttle_policy(
        self,
        policy: ThermalThrottleProfile,
        temporal=False,
        notify: bool = True,
        force=False,
    ):
        with self.lock:
            policy_name = policy.name
            power_profile = self.throttle_power_assoc[policy]

            if self.thermal_throttle_profile != policy or force:
                try:
                    boost_enabled = self.throttle_boost_assoc[policy]
                    no_boost_reason = None
                    if configuration.settings.password is None:
                        no_boost_reason = "missing password"
                    elif self.boost_control is None:
                        no_boost_reason = "unsupported"

                    self.logger.info(f"Setting {policy_name.lower()} profile")
                    self.logger.addTab()

                    self.logger.info(f"Throttle policy: {policy_name}")
                    platform_client.throttleThermalPolicy = policy
                    self.thermal_throttle_profile = policy

                    self.logger.info(f"Fan curve: {policy_name}")
                    fan_curves_client.setCurvesToDefaults(policy)
                    fan_curves_client.resetProfileCurves(policy)
                    fan_curves_client.setFanCurvesEnabled(policy, True)

                    self.logger.info(f"Power policy: {power_profile.name}")
                    power_profile_client.activeProfile = power_profile

                    if no_boost_reason is not None:
                        self.logger.info(f"Boost: omitted due to {no_boost_reason}")
                    else:
                        if boost_enabled:
                            self.logger.info("Boost: Enabled")
                        else:
                            self.logger.info("Boost: Disabled")

                        if (
                            self.boost_control is not None
                            and boost_enabled != self.last_boost
                        ):
                            target = "on" if boost_enabled else "off"
                            value = self.boost_control[target]
                            path = self.boost_control["path"]

                            command = f'echo "{cryptography.decrypt_string(configuration.settings.password)}" | sudo -S bash -c "echo \'{value}\' | tee {path}" &>> /dev/null'
                            subprocess.run(command, shell=True, check=True)
                    self.logger.remTab()
                    self.logger.info("Profile setted succesfully")

                    configuration.platform.profiles.last = policy.value
                    configuration.save_config()

                    if notify:
                        notifier.show_toast(
                            translator.translate(
                                "profile.applied",
                                {
                                    "profile": translator.translate(
                                        f"label.profile.{policy_name}"
                                    ).lower()
                                },
                            )
                        )
                except Exception as error:
                    self.logger.error(f"Couldn't set profile: {error}")
                    self.logger.remTab()

    def set_battery_threshold(self, value: BatteryThreshold):
        if value != self.battery_charge_limit:
            platform_client.chargeControlEndThreshold = value
            notifier.show_toast(
                translator.translate(
                    "applied.battery.threshold", {"value": value.value}
                )
            )


platform_service = PlatformService()
