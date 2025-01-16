from enum import IntEnum


class ThermalThrottleProfile(IntEnum):
    """Thermal throttle policy enum"""

    PERFORMANCE = (1,)
    BALANCED = (0,)
    QUIET = 2


def get_next_thermal_throttle_profile(current: ThermalThrottleProfile):
    """Get next profile"""
    if current == ThermalThrottleProfile.PERFORMANCE:
        return ThermalThrottleProfile.QUIET
    if current == ThermalThrottleProfile.QUIET:
        return ThermalThrottleProfile.BALANCED
    if current == ThermalThrottleProfile.BALANCED:
        return ThermalThrottleProfile.PERFORMANCE

    return current
