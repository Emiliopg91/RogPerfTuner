import math

from lib.clients.openrgb.effects.base.abstract_effect import AbstractEffect
from lib.utils.singleton import singleton
from lib.utils.openrgb import OpenRGBUtils


@singleton
class BreathingEffect(AbstractEffect):
    """Breathing effect"""

    def __init__(self):
        super().__init__("Breathing", "#FF0000")
        self._frequency = 0.75

    def apply_effect(self):
        offset = 0
        while self._is_running:
            factor = abs(math.sin(offset * self._frequency))
            new_color = OpenRGBUtils.dim(self._color, factor)

            for dev in self._devices:
                self._set_colors(dev, [new_color] * len(dev.colors))

            if round(offset * 10) == 42:
                offset = 0
            else:
                offset += 0.1

            self._sleep(0.075)


breathing_effect = BreathingEffect()
