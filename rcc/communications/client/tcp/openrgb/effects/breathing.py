import math

from rcc.communications.client.tcp.openrgb.effects.base.abstract_effect import AbstractEffect
from framework.singleton import singleton


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
            new_color = self._color.dim(self._sin_steps[offset])

            for dev in self._devices:
                if dev.enabled:
                    self._set_colors(dev, [new_color] * len(dev.colors))

            offset = (offset + 1) % self._step_count

            if offset > 0:
                self._sleep(self._frequency)
            else:
                self._sleep(self._pause_time)


breathing_effect = BreathingEffect()
