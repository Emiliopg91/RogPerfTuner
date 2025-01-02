import math

from lib.clients.openrgb.effects.base.abstract_effect import AbstractEffect
from lib.utils.singleton import singleton
from lib.utils.openrgb import OpenRGBUtils


@singleton
class BreathingEffect(AbstractEffect):
    """Breathing effect"""

    def __init__(self):
        super().__init__("Breathing")
        self.frequency = 0.75

    def apply_effect(self):
        offset = 0
        hue = -102
        while self.is_running:
            factor = abs(math.sin(offset * self.frequency))

            if offset == 0:
                hue += 102
                self._color = OpenRGBUtils.from_hsv(hue, 1, 1)

            new_color = OpenRGBUtils.dim(self._color, factor)

            for dev in self.devices:
                self._set_colors(dev, [new_color] * len(dev.colors))

            if round(offset * 10) == 42:
                offset = 0
            else:
                offset += 0.1

            self._sleep(0.075)


breathing_effect = BreathingEffect()
