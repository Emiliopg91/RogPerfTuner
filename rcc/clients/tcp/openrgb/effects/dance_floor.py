import random

from rcc.clients.tcp.openrgb.client.utils import RGBColor
from rcc.clients.tcp.openrgb.effects.base.abstract_effect import AbstractEffect
from rcc.utils.singleton import singleton


@singleton
class DanceFloor(AbstractEffect):
    """Dance floor effect"""

    def __init__(self):
        super().__init__("Dance floor")

    def _get_random(self, length):
        randoms = []
        for _ in range(length):
            randoms.append(RGBColor.fromHSV(random.randint(0, 359), random.randint(0, 25) + 75, 100))
        return randoms

    def apply_effect(self):
        while self._is_running:
            for dev in self._devices:
                self._set_colors(dev, self._get_random(len(dev.colors)))
            self._sleep(0.5)  # 500 ms


dance_floor = DanceFloor()
