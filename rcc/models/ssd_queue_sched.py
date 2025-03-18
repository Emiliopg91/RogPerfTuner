from enum import StrEnum


class SsdQueueScheduler(StrEnum):
    """Enum for ssd queue scheduler"""

    NOOP = "none"
    MQ_DEADLINE = "mq-deadline"
