from enum import IntEnum


class MangoHudLevel(IntEnum):
    """Enum for mangohud level"""

    NO_DISPLAY = 0
    FPS_ONLY = 1
    HORIZONTAL_VIEW = 2
    EXTENDED = 3
    HIGH_DETAILED = 4
