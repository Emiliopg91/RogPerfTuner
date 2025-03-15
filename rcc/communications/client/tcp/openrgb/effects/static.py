from framework.singleton import singleton
from rcc.communications.client.tcp.openrgb.effects.base.abstract_effect import AbstractEffect


@singleton
class StaticEffect(AbstractEffect):
    """Static effect"""

    def __init__(self):
        super().__init__("Static", "#FF0000")

    def apply_effect(self):
        while self._is_running:
            for dev in self._devices:
                self._set_colors(dev, [self._color] * len(dev.leds))
            self._sleep(0.3)


STATIC_EFFECT = StaticEffect()
