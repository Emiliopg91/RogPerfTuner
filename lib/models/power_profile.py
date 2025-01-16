from enum import StrEnum


class PowerProfile(StrEnum):
    """Power profile enum"""

    POWER_SAVER = ("power-saver",)
    BALANCED = ("balanced",)
    PERFORMANCE = "performance"
