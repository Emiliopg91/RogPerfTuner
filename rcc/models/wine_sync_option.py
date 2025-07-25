from enum import StrEnum


class WineSyncOption(StrEnum):
    """Enum for mangohud level"""

    AUTO = "auto"
    NTSYNC = "ntsync"
    FSYNC = "fsync"
    ESYNC = "esync"
    NONE = "none"
