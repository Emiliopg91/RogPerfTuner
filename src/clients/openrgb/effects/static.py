from .base.abstract_effect import AbstractEffect
from ....utils.singleton import singleton


@singleton
class StaticEffect(AbstractEffect):
    def __init__(self):
        super().__init__("Static", "#FF0000")

    def apply_effect(self):
        for dev in self.devices:
            self.set_colors(dev, [self._color] * len(dev.leds))


static_effect = StaticEffect()
