from .base.abstract_effect import AbstractEffect
from ..sdk.utils import RGBColor
from ....utils.singleton import singleton

import math
import random


@singleton
class BreathingEffect(AbstractEffect):
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
                self._color = RGBColor.fromHSV(hue, 1, 1)

            new_color = self._color.dim(factor)

            for dev in self.devices:
                self.set_colors(dev, [new_color] * len(dev.colors))

            if round(offset * 10) == 42:
                offset = 0
            else:
                offset += 0.1

            self.sleep(0.075)


breathing_effect = BreathingEffect()
