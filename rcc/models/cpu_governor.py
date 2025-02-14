from enum import StrEnum


class CpuGovernor(StrEnum):
    """Power profile enum"""

    POWERSAVER = "powersaver"
    PERFORMANCE = "performance"
