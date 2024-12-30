from enum import IntEnum


class ThermalThrottleProfile(IntEnum):
    PERFORMANCE = (1,)
    BALANCED = (0,)
    QUIET = 2


def getNextThermalThrottleProfile(current: ThermalThrottleProfile):
    if current == ThermalThrottleProfile.PERFORMANCE:
        return ThermalThrottleProfile.QUIET
    elif current == ThermalThrottleProfile.QUIET:
        return ThermalThrottleProfile.BALANCED
    elif current == ThermalThrottleProfile.BALANCED:
        return ThermalThrottleProfile.PERFORMANCE
