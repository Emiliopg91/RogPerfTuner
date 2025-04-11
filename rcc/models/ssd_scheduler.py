from enum import StrEnum


class SsdScheduler(StrEnum):
    """Enum for ssd queue scheduler"""

    NOOP = "none"
    MQ_DEADLINE = "mq-deadline"
