from enum import IntEnum


class PlatformProfile(IntEnum):
    """Thermal throttle policy enum"""

    PERFORMANCE = (1,)
    BALANCED = (0,)
    QUIET = 2
