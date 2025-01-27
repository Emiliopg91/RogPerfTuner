from enum import IntEnum

from rcc.models.platform_profile import PlatformProfile
from rcc.models.power_profile import PowerProfile


class PerformanceProfile(IntEnum):
    """Performance profile enum"""

    QUIET = 0
    BALANCED = 1
    PERFORMANCE = 2

    def get_boost_enabled(self) -> bool:
        """Get if profile enables boost"""
        return self == PerformanceProfile.PERFORMANCE

    def get_platform_profile(self) -> PlatformProfile:
        """Get associated platform profile"""
        if self == PerformanceProfile.QUIET:
            return PlatformProfile.QUIET
        if self == PerformanceProfile.BALANCED:
            return PlatformProfile.BALANCED

        return PlatformProfile.PERFORMANCE

    def get_power_profile(self) -> PowerProfile:
        """Get associated power profile"""
        if self == PerformanceProfile.QUIET:
            return PowerProfile.POWER_SAVER

        return PowerProfile.PERFORMANCE

    def get_greater(self, other):
        """Get the profile with higher performance"""
        if PerformanceProfile.PERFORMANCE in (self, other):
            return PerformanceProfile.PERFORMANCE

        if PerformanceProfile.BALANCED in (self, other):
            return PerformanceProfile.BALANCED

        return PerformanceProfile.QUIET

    def get_next_performance_profile(self):
        """Get next profile"""
        if self == PerformanceProfile.PERFORMANCE:
            return PerformanceProfile.QUIET
        if self == PerformanceProfile.BALANCED:
            return PerformanceProfile.PERFORMANCE
        if self == PerformanceProfile.QUIET:
            return PerformanceProfile.BALANCED

        return self
