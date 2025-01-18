import math

from lib.clients.tcp.openrgb.effects.base.abstract_effect import AbstractEffect
from lib.utils.singleton import singleton
from lib.utils.openrgb import OpenRGBUtils


@singleton
class BreathingEffect(AbstractEffect):
    """Breathing effect"""

    def __init__(self):
        super().__init__("Breathing", "#FF0000")
        self._total_time = 4
        self._pause_time = 1
        self._frequency = 0.05
        self._step_count = int((self._total_time - self._pause_time) / self._frequency)
        self._sin_steps = [math.sin(math.pi * (offset / (self._step_count - 1))) for offset in range(self._step_count)]

    def apply_effect(self):
        offset = 0
        while self._is_running:
            new_color = OpenRGBUtils.dim(self._color, self._sin_steps[offset])

            for dev in self._devices:
                self._set_colors(dev, [new_color] * len(dev.colors))

            offset = (offset + 1) % self._step_count

            if offset > 0:
                self._sleep(self._frequency)
            else:
                self._sleep(self._pause_time)


breathing_effect = BreathingEffect()
