from .base.abstract_effect import AbstractEffect
from ..sdk.utils import RGBColor
from ....utils.singleton import singleton

import random
import time

@singleton
class DanceFloor(AbstractEffect):
    def __init__(self):
        super().__init__("Dance floor")

    def get_random(self, length):
        randoms = []
        for _ in range(length):
            randoms.append(RGBColor.fromHSV(random.random() * 359, random.random() * 0.25 + 0.75, 1))
        return randoms

    def apply_effect(self):
        while self.is_running:
            for dev in self.devices:
                self.set_colors(dev, self.get_random(len(dev.colors)))
            self.sleep(0.5)  # 500 ms


dance_floor = DanceFloor()