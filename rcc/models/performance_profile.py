from enum import IntEnum

from rcc.communications.client.dbus.asus.armoury.nvidia.nv_boost_client import NV_BOOST_CLIENT
from rcc.communications.client.dbus.asus.armoury.intel.pl1_spl_client import PL1_SPL_CLIENT
from rcc.communications.client.dbus.asus.armoury.intel.pl2_sppt_client import PL2_SPPT_CLIENT
from rcc.models.platform_profile import PlatformProfile
from rcc.models.power_profile import PowerProfile


class PerformanceProfile(IntEnum):
    """Performance profile enum"""

    QUIET = 0
    BALANCED = 1
    PERFORMANCE = 2

    @property
    def pl1_spl(self):
        """Get PL1  SPL Watts value"""
        if not PL1_SPL_CLIENT.available:
            return None

        if self == PerformanceProfile.PERFORMANCE:
            return int(PL1_SPL_CLIENT.max_value * 0.7)
        if self == PerformanceProfile.BALANCED:
            return int(PL1_SPL_CLIENT.max_value * 0.5)
        if self == PerformanceProfile.QUIET:
            return int(PL1_SPL_CLIENT.max_value * 0.3)

        return PL1_SPL_CLIENT.current_value

    @property
    def pl2_sppt(self):
        """Get PL2 SPPT Watts value"""
        if not PL2_SPPT_CLIENT.available:
            return None

        if self == PerformanceProfile.PERFORMANCE:
            return int(PL2_SPPT_CLIENT.max_value * 0.9)
        if self == PerformanceProfile.BALANCED:
            return int(PL2_SPPT_CLIENT.max_value * 0.75)
        if self == PerformanceProfile.QUIET:
            return int(PL2_SPPT_CLIENT.max_value * 0.45)

        return PL1_SPL_CLIENT.current_value

    @property
    def nv_boost(self):
        """Get NV dynamic boost Watts value"""
        if not PL2_SPPT_CLIENT.available:
            return None

        if self == PerformanceProfile.PERFORMANCE:
            return NV_BOOST_CLIENT.max_value
        if self == PerformanceProfile.BALANCED:
            return int((NV_BOOST_CLIENT.max_value + NV_BOOST_CLIENT.min_value) / 2)
        if self == PerformanceProfile.QUIET:
            return NV_BOOST_CLIENT.min_value

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

        return PowerProfile.PERFORMANCE

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

    def get_greater(self, other):
        """Get the profile with higher performance"""
        if PerformanceProfile.PERFORMANCE in (self, other):
            return PerformanceProfile.PERFORMANCE

        if PerformanceProfile.BALANCED in (self, other):
            return PerformanceProfile.BALANCED

        return PerformanceProfile.QUIET
