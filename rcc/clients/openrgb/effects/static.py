from rcc.clients.openrgb.effects.base.abstract_effect import AbstractEffect
from rcc.utils.singleton import singleton


@singleton
class StaticEffect(AbstractEffect):
    """Static effect"""

    def __init__(self):
        super().__init__("Static", "#FF0000")

    def apply_effect(self):
        for dev in self._devices:
            self._set_colors(dev, [self._color] * len(dev.leds))


static_effect = StaticEffect()
