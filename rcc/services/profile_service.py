import time
from threading import Lock


from framework.logger import Logger, logged_method
from framework.singleton import singleton
from rcc.communications.client.cmd.linux.cpupower_client import CPU_POWER_CLIENT
from rcc.communications.client.dbus.asus.core.fan_curves_client import FAN_CURVES_CLIENT
from rcc.communications.client.dbus.asus.core.platform_client import PLATFORM_CLIENT
from rcc.communications.client.cmd.linux.power_profiles_client import POWER_PROFILE_CLIENT
from rcc.communications.client.dbus.linux.upower_client import UPOWER_CLIENT
from rcc.gui.notifier import NOTIFIER
from rcc.models.cpu_governor import CpuGovernor
from rcc.models.fan_curve import FanCurve
from rcc.models.performance_profile import PerformanceProfile
from rcc.models.platform_profile import PlatformProfile
from rcc.models.power_profile import PowerProfile
from rcc.models.ssd_scheduler import SsdScheduler
from rcc.services.hardware_service import HARDWARE_SERVICE
from rcc.utils.beans import EVENT_BUS, TRANSLATOR
from rcc.utils.configuration import CONFIGURATION
from rcc.utils.events import HARDWARE_SERVICE_ON_BATTERY, PLATFORM_SERVICE_PROFILE_CHANGED, STEAM_SERVICE_GAME_EVENT


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

        if UPOWER_CLIENT.available:
            self.on_bat = UPOWER_CLIENT.on_battery

        if PLATFORM_CLIENT.available:
            self._platform_profile = PLATFORM_CLIENT.platform_profile
            self._battery_charge_limit = PLATFORM_CLIENT.charge_control_end_threshold

            PLATFORM_CLIENT.change_platform_profile_on_ac = False
            PLATFORM_CLIENT.change_platform_profile_on_battery = False
            PLATFORM_CLIENT.platfom_profile_linked_epp = True

        self.__curves: dict[PlatformProfile, list[FanCurve]] = {}

        if FAN_CURVES_CLIENT.available:
            self._logger.info("Configuring improved fan curves")
            for p in PlatformProfile:
                FAN_CURVES_CLIENT.set_curves_to_defaults(p)
                FAN_CURVES_CLIENT.reset_profile_curves(p)
                time.sleep(0.2)

                curves = FAN_CURVES_CLIENT.fan_curve_data(p)
                for curve in curves:
                    curve.enabled = True
                    for i in range(len(curve.points)):  # pylint: disable=consider-using-enumerate
                        curve.points[i] = (curve.points[i][0], min(100, round(curve.points[i][1] * 1.25)))
                self.__curves[p] = curves

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
            self.set_performance_profile(PerformanceProfile.QUIET, True, True)

    @logged_method
    def set_performance_profile(  # pylint: disable=too-many-locals
        self, profile: PerformanceProfile, temporal=False, force=False
    ) -> None:
        """Establish performance profile"""
        with self._lock:
            profile_name = profile.name
            if self._performance_profile != profile or force:
                platform_profile = profile.platform_profile
                power_profile = profile.power_profile
                cpu_governor = profile.battery_governor if self.on_bat else profile.ac_governor
                boost_enabled = profile.battery_boost if self.on_bat else profile.ac_boost
                ssd_sched = profile.ssd_queue_scheduler
                try:
                    self._logger.info(f"Setting {profile_name} profile")  # pylint: disable=line-too-long
                    t0 = time.time()
                    self._logger.add_tab()

                    self.__set_throttle_policy(profile, platform_profile)
                    self.__set_tdp_values(profile)
                    self.__set_fan_curves(platform_profile)
                    self.__set_boost(boost_enabled)
                    self.__set_cpu_governor(cpu_governor, power_profile)
                    self.__set_ssd_scheduler(ssd_sched)

                    self._logger.rem_tab()
                    self._logger.info(f"Profile setted succesfully after {round(1000*(time.time()-t0))/1000} ms")

                    if not temporal and not self.on_bat:
                        CONFIGURATION.platform.profiles.profile = profile.value
                        CONFIGURATION.save_config()

                    NOTIFIER.show_toast(
                        TRANSLATOR.translate(
                            "profile.applied",
                            {"profile": TRANSLATOR.translate(f"label.profile.{profile_name}").lower()},
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
            self.set_performance_profile(PerformanceProfile(CONFIGURATION.platform.profiles.profile), True, True)
            self._logger.rem_tab()

    def __set_throttle_policy(self, profile: PerformanceProfile, platform_profile: PlatformProfile):
        try:
            platform_profile = profile.platform_profile
            self._logger.info(f"Throttle policy: {platform_profile.name}")
            self._logger.add_tab()

            if PLATFORM_CLIENT.available:
                self._platform_profile = platform_profile
                PLATFORM_CLIENT.platform_profile = platform_profile
                PLATFORM_CLIENT.enable_ppt_group = True

            self._logger.rem_tab()
        except Exception as e:
            self._logger.error(f"Error while setting platform policy: {e}")

    def __set_tdp_values(self, profile: PerformanceProfile):
        if HARDWARE_SERVICE.tdp_available:
            try:
                self._logger.info("TDP values")
                self._logger.add_tab()
                HARDWARE_SERVICE.set_cpu_tdp(profile)
                HARDWARE_SERVICE.set_gpu_tgp(profile)
                self._logger.rem_tab()
            except Exception as e:
                self._logger.error(f"Error while setting TDP values: {e}")

    def __set_fan_curves(self, platform_profile: PlatformProfile):
        if FAN_CURVES_CLIENT.available:
            try:
                self._logger.info(f"Fan curve: {platform_profile.name}")
                for curve in self.__curves[platform_profile]:
                    FAN_CURVES_CLIENT.set_fan_curve(platform_profile, curve)
            except Exception as e:
                self._logger.error(f"Error while setting fan curve: {e}")

    def __set_cpu_governor(self, governor: CpuGovernor, power_profile: PowerProfile):
        if CPU_POWER_CLIENT.available:
            try:
                self._logger.info(f"CPU governor: {governor.name}")
                CPU_POWER_CLIENT.set_governor(governor)
                time.sleep(0.25)
                if POWER_PROFILE_CLIENT.available:
                    self._logger.info(f"Power profile: {power_profile.name}")
                    POWER_PROFILE_CLIENT.active_profile = power_profile
            except Exception as e:
                self._logger.error(f"Error while setting CPU governor: {e}")

    def __set_boost(self, boost_enabled: bool):
        try:
            HARDWARE_SERVICE.set_boost_status(boost_enabled)
        except Exception as e:
            self._logger.error(f"Error while setting boost mode: {e}")

    def __set_ssd_scheduler(self, scheduler: SsdScheduler):
        try:
            HARDWARE_SERVICE.set_ssd_scheduler(scheduler)
        except Exception as e:
            self._logger.error(f"Error while setting SSD scheduler: {e}")


PROFILE_SERVICE = ProfileService()
