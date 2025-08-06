from enum import IntEnum


class PlatformProfile(IntEnum):
    """Thermal throttle policy enum"""

    BALANCED = 0
    PERFORMANCE = 1
    LOW_POWER = 3
