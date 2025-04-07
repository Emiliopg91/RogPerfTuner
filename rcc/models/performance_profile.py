from enum import IntEnum

from rcc.communications.client.dbus.asus.armoury.nvidia.nv_boost_client import NV_BOOST_CLIENT
from rcc.communications.client.dbus.asus.armoury.intel.pl1_spl_client import PL1_SPL_CLIENT
from rcc.communications.client.dbus.asus.armoury.intel.pl2_sppt_client import PL2_SPPT_CLIENT
from rcc.communications.client.dbus.asus.armoury.nvidia.nv_temp_client import NV_TEMP_CLIENT
from rcc.models.cpu_governor import CpuGovernor
from rcc.models.platform_profile import PlatformProfile
from rcc.models.power_profile import PowerProfile
from rcc.models.ssd_scheduler import SsdScheduler


class PerformanceProfile(IntEnum):
    """Performance profile enum"""

    QUIET = 0
    BALANCED = 1
    PERFORMANCE = 2

    def __ac_tdp_to_bat_tdp(self, tdp: int, min_tdp: int):
        return max(min_tdp, round(tdp * 0.6))

    @property
    def ac_intel_pl1_spl(self):
        """Get AC PL1 SPL Watts value"""
        if not PL1_SPL_CLIENT.available:
            return None

        if self == PerformanceProfile.PERFORMANCE:
            return int(PL1_SPL_CLIENT.max_value * 1)
        if self == PerformanceProfile.BALANCED:
            return int(PL1_SPL_CLIENT.max_value * 0.6)
        if self == PerformanceProfile.QUIET:
            return int(PL1_SPL_CLIENT.max_value * 0.4)

        return PL1_SPL_CLIENT.current_value

    @property
    def battery_intel_pl1_spl(self):
        """Get battery PL1 SPL Watts value"""
        if not PL1_SPL_CLIENT.available:
            return None
        return self.__ac_tdp_to_bat_tdp(self.ac_intel_pl1_spl, PL1_SPL_CLIENT.min_value)

    @property
    def ac_intel_pl2_sppt(self):
        """Get AC PL2 SPPT Watts value"""
        if not PL2_SPPT_CLIENT.available:
            return None

        if not self.ac_boost:
            return self.ac_intel_pl1_spl

        if self == PerformanceProfile.PERFORMANCE:
            return int(PL2_SPPT_CLIENT.max_value * 1)
        if self == PerformanceProfile.BALANCED:
            return int(PL2_SPPT_CLIENT.max_value * 0.8)
        if self == PerformanceProfile.QUIET:
            return int(PL2_SPPT_CLIENT.max_value * 0.6)

        return PL2_SPPT_CLIENT.current_value

    @property
    def battery_intel_pl2_sppt(self):
        """Get battery PL2 SPPT Watts value"""
        if not PL2_SPPT_CLIENT.available:
            return None

        if not self.battery_boost:
            return self.battery_intel_pl1_spl

        return self.__ac_tdp_to_bat_tdp(self.ac_intel_pl2_sppt, PL2_SPPT_CLIENT.min_value)

    @property
    def ac_nv_boost(self):
        """Get AC NV dynamic boost Watts value"""
        if not NV_BOOST_CLIENT.available:
            return None

        if self == PerformanceProfile.PERFORMANCE:
            return NV_BOOST_CLIENT.max_value
        if self == PerformanceProfile.BALANCED:
            return int((NV_BOOST_CLIENT.max_value + NV_BOOST_CLIENT.min_value) / 2)
        if self == PerformanceProfile.QUIET:
            return NV_BOOST_CLIENT.min_value

        return NV_BOOST_CLIENT.current_value

    @property
    def battery_nv_boost(self):
        """Get battery NV dynamic boost Watts value"""
        if not NV_BOOST_CLIENT.available:
            return None
        return self.__ac_tdp_to_bat_tdp(self.ac_nv_boost, NV_BOOST_CLIENT.min_value)

    @property
    def ac_nv_temp(self):
        """Get AC NV temperature limit"""
        if not NV_TEMP_CLIENT.available:
            return None

        return NV_TEMP_CLIENT.max_value

    @property
    def battery_nv_temp(self):
        """Get battery NV temperature limit"""
        if not NV_TEMP_CLIENT.available:
            return None

        if self == PerformanceProfile.PERFORMANCE:
            return NV_TEMP_CLIENT.max_value
        if self == PerformanceProfile.BALANCED:
            return int((NV_TEMP_CLIENT.max_value + NV_TEMP_CLIENT.min_value) / 2)
        if self == PerformanceProfile.QUIET:
            return NV_TEMP_CLIENT.min_value

        return NV_BOOST_CLIENT.current_value

    @property
    def platform_profile(self) -> PlatformProfile:
        """Get associated platform profile"""
        if self == PerformanceProfile.QUIET:
            return PlatformProfile.QUIET
        if self == PerformanceProfile.BALANCED:
            return PlatformProfile.BALANCED

        return PlatformProfile.PERFORMANCE

    @property
    def power_profile(self) -> PowerProfile:
        """Get associated power profile"""
        if self == PerformanceProfile.QUIET:
            return PowerProfile.POWER_SAVER
        if self == PerformanceProfile.BALANCED:
            return PowerProfile.BALANCED
        return PowerProfile.PERFORMANCE

    @property
    def ssd_queue_scheduler(self) -> SsdScheduler:
        """Get associated power profile"""
        if self == PerformanceProfile.QUIET:
            return SsdScheduler.NOOP
        return SsdScheduler.MQ_DEADLINE

    @property
    def next_performance_profile(self):
        """Get next profile"""
        if self == PerformanceProfile.PERFORMANCE:
            return PerformanceProfile.QUIET
        if self == PerformanceProfile.BALANCED:
            return PerformanceProfile.PERFORMANCE
        if self == PerformanceProfile.QUIET:
            return PerformanceProfile.BALANCED

        return self

    @property
    def ac_boost(self):
        """Boost on battery"""
        return True

    @property
    def battery_boost(self):
        """Boost on battery"""
        return False

    @property
    def ac_governor(self):
        """AC Governor"""
        return CpuGovernor.PERFORMANCE if self == PerformanceProfile.PERFORMANCE else CpuGovernor.POWERSAVE

    @property
    def battery_governor(self):
        """Battery Governor"""
        return CpuGovernor.POWERSAVE

    def get_greater(self, other):
        """Get the profile with higher performance"""
        if PerformanceProfile.PERFORMANCE in (self, other):
            return PerformanceProfile.PERFORMANCE

        if PerformanceProfile.BALANCED in (self, other):
            return PerformanceProfile.BALANCED

        return PerformanceProfile.QUIET
