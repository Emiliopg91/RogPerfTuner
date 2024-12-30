from enum import IntEnum


class RgbBrightness(IntEnum):
    MAX = 4
    HIGH = 3
    MEDIUM = 2
    LOW = 1
    OFF = 0


def getNextBrightness(brightness: RgbBrightness) -> RgbBrightness:
    if brightness == RgbBrightness.HIGH:
        return RgbBrightness.MAX
    elif brightness == RgbBrightness.MEDIUM:
        return RgbBrightness.HIGH
    elif brightness == RgbBrightness.LOW:
        return RgbBrightness.MEDIUM
    elif brightness == RgbBrightness.OFF:
        return RgbBrightness.LOW
    else:
        return brightness


def getPreviousBrightness(brightness: RgbBrightness) -> RgbBrightness:
    if brightness == RgbBrightness.MAX:
        return RgbBrightness.HIGH
    elif brightness == RgbBrightness.HIGH:
        return RgbBrightness.MEDIUM
    elif brightness == RgbBrightness.MEDIUM:
        return RgbBrightness.LOW
    elif brightness == RgbBrightness.LOW:
        return RgbBrightness.OFF
    else:
        return brightness
