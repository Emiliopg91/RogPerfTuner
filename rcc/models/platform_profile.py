from enum import IntEnum


class PlatformProfile(IntEnum):
    """Thermal throttle policy enum"""

    PERFORMANCE = (1,)
    BALANCED = (0,)
    QUIET = 2


def get_greater(p1: PlatformProfile, p2: PlatformProfile):
    """Get the profile with higher performance"""
    if PlatformProfile.PERFORMANCE in (p1, p2):
        return PlatformProfile.PERFORMANCE

    if PlatformProfile.BALANCED in (p1, p2):
        return PlatformProfile.BALANCED

    return PlatformProfile.QUIET


def get_next_thermal_throttle_profile(current: PlatformProfile):
    """Get next profile"""
    if current == PlatformProfile.PERFORMANCE:
        return PlatformProfile.QUIET
    if current == PlatformProfile.QUIET:
        return PlatformProfile.BALANCED
    if current == PlatformProfile.BALANCED:
        return PlatformProfile.PERFORMANCE

    return current
