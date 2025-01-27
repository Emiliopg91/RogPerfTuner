from enum import IntEnum


class RgbBrightness(IntEnum):
    """RGB Brightness enum"""

    MAX = 4
    HIGH = 3
    MEDIUM = 2
    LOW = 1
    OFF = 0

    def get_next_brightness(self):
        """Get the following brightness level"""
        if self == RgbBrightness.HIGH:
            return RgbBrightness.MAX
        if self == RgbBrightness.MEDIUM:
            return RgbBrightness.HIGH
        if self == RgbBrightness.LOW:
            return RgbBrightness.MEDIUM
        if self == RgbBrightness.OFF:
            return RgbBrightness.LOW

        return self

    def get_previous_brightness(self):
        """Get the previous brightness level"""
        if self == RgbBrightness.MAX:
            return RgbBrightness.HIGH
        if self == RgbBrightness.HIGH:
            return RgbBrightness.MEDIUM
        if self == RgbBrightness.MEDIUM:
            return RgbBrightness.LOW
        if self == RgbBrightness.LOW:
            return RgbBrightness.OFF

        return self
