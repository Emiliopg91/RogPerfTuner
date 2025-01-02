from enum import IntEnum


class RgbBrightness(IntEnum):
    """RGB Brightness enum"""

    MAX = 4
    HIGH = 3
    MEDIUM = 2
    LOW = 1
    OFF = 0


def get_next_brightness(brightness: RgbBrightness) -> RgbBrightness:
    """Get the following brightness level"""
    if brightness == RgbBrightness.HIGH:
        return RgbBrightness.MAX
    if brightness == RgbBrightness.MEDIUM:
        return RgbBrightness.HIGH
    if brightness == RgbBrightness.LOW:
        return RgbBrightness.MEDIUM
    if brightness == RgbBrightness.OFF:
        return RgbBrightness.LOW

    return brightness


def get_previous_brightness(brightness: RgbBrightness) -> RgbBrightness:
    """Get the previous brightness level"""
    if brightness == RgbBrightness.MAX:
        return RgbBrightness.HIGH
    if brightness == RgbBrightness.HIGH:
        return RgbBrightness.MEDIUM
    if brightness == RgbBrightness.MEDIUM:
        return RgbBrightness.LOW
    if brightness == RgbBrightness.LOW:
        return RgbBrightness.OFF

    return brightness
