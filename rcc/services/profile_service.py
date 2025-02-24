import concurrent
import time
from threading import Lock


from framework.logger import Logger, logged_method
from framework.singleton import singleton
from rcc.communications.client.dbus.asus.core.fan_curves_client import FAN_CURVES_CLIENT
from rcc.communications.client.dbus.asus.core.platform_client import PLATFORM_CLIENT
from rcc.communications.client.dbus.linux.power_profiles_client import POWER_PROFILE_CLIENT
from rcc.communications.client.dbus.linux.upower_client import UPOWER_CLIENT
from rcc.gui.notifier import NOTIFIER
from rcc.models.cpu_governor import CpuGovernor
from rcc.models.performance_profile import PerformanceProfile
from rcc.models.platform_profile import PlatformProfile
from rcc.models.power_profile import PowerProfile
from rcc.services.hardware_service import HARDWARE_SERVICE
from rcc.utils.beans import EVENT_BUS, TRANSLATOR
from rcc.utils.configuration import CONFIGURATION
from rcc.utils.events import HARDWARE_SERVICE_ON_BATTERY, PLATFORM_SERVICE_PROFILE_CHANGED, STEAM_SERVICE_GAME_EVENT
from rcc.utils.shell import SHELL


@singleton
class ProfileService:
    """Service for platform setting"""

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing ProfileService")
        self._logger.add_tab()
        self._lock = Lock()
        self._ac_events_enabled = True

        self._performance_profile = PerformanceProfile(CONFIGURATION.platform.profiles.profile)
        self._platform_profile = PLATFORM_CLIENT.platform_profile
        self._battery_charge_limit = PLATFORM_CLIENT.charge_control_end_threshold
        self.on_bat = UPOWER_CLIENT.on_battery

        PLATFORM_CLIENT.change_platform_profile_on_ac = False
        PLATFORM_CLIENT.change_platform_profile_on_battery = False

        EVENT_BUS.on(HARDWARE_SERVICE_ON_BATTERY, self._on_ac_battery_change)
        EVENT_BUS.on(STEAM_SERVICE_GAME_EVENT, self._on_game_event)

        self._logger.rem_tab()

    @property
    def performance_profile(self) -> PerformanceProfile:
        """Performance profile"""
        return self._performance_profile

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
            time.sleep(0.05)
            platform_profile = profile.platform_profile
            self._logger.info(f"Throttle policy: {platform_profile.name}")
            self._logger.add_tab()
            self._platform_profile = platform_profile
            PLATFORM_CLIENT.platform_profile = platform_profile

            PLATFORM_CLIENT.enable_ppt_group = True
            time.sleep(0.05)

            HARDWARE_SERVICE.set_cpu_tdp(profile)
            HARDWARE_SERVICE.set_gpu_tgp(profile)

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
            HARDWARE_SERVICE.set_boost_status(boost_enabled)
        except Exception as e:
            self._logger.error(f"Error while setting boost mode: {e}")


PROFILE_SERVICE = ProfileService()
