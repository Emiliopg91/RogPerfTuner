import random

from rcc.clients.tcp.openrgb.effects.base.abstract_effect import AbstractEffect
from rcc.utils.singleton import singleton
from rcc.utils.openrgb import OpenRGBUtils


@singleton
class DanceFloor(AbstractEffect):
    """Dance floor effect"""

    def __init__(self):
        super().__init__("Dance floor")

    def _get_random(self, length):
        randoms = []
        for _ in range(length):
            randoms.append(OpenRGBUtils.from_hsv(random.randint(0, 359), random.randint(0, 25) / 100 + 0.75, 1))
        return randoms

    def apply_effect(self):
        while self._is_running:
            for dev in self._devices:
                self._set_colors(dev, self._get_random(len(dev.colors)))
            self._sleep(0.5)  # 500 ms


dance_floor = DanceFloor()
