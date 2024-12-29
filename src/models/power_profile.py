from enum import StrEnum


class PowerProfile(StrEnum):
    POWER_SAVER = ("power-saver",)
    BALANCED = ("balanced",)
    PERFORMANCE = "performance"
