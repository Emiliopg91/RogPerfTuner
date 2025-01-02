import random

from lib.clients.openrgb.effects.base.abstract_effect import AbstractEffect
from lib.utils.singleton import singleton
from lib.utils.openrgb import OpenRGBUtils


@singleton
class DanceFloor(AbstractEffect):
    """Dance floor effect"""

    def __init__(self):
        super().__init__("Dance floor")

    def _get_random(self, length):
        randoms = []
        for _ in range(length):
            randoms.append(
                OpenRGBUtils.from_hsv(
                    random.random() * 359, random.random() * 0.25 + 0.75, 1
                )
            )
        return randoms

    def apply_effect(self):
        while self.is_running:
            for dev in self.devices:
                self._set_colors(dev, self._get_random(len(dev.colors)))
            self._sleep(0.5)  # 500 ms


dance_floor = DanceFloor()
