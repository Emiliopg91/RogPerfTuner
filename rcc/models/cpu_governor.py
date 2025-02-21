from enum import StrEnum


class CpuGovernor(StrEnum):
    """Power profile enum"""

    POWERSAVE = "powersave"
    PERFORMANCE = "performance"
