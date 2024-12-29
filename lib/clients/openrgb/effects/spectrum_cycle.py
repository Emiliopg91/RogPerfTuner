from .base.abstract_effect import AbstractEffect
from ..sdk.utils import RGBColor
from ....utils.singleton import singleton

import time

@singleton
class SpectrumCycle(AbstractEffect):
    def __init__(self):
        super().__init__("Spectrum cycle")

    def apply_effect(self):
        offset = 0
        while self.is_running:
            for dev in self.devices:
                colors = [RGBColor.fromHSV(offset, 1, 1)] * len(dev.leds)
                self.set_colors(dev, colors)
            offset = (offset + 1) % 360
            self.sleep(0.02)  # 20 ms

spectrum_cycle = SpectrumCycle()