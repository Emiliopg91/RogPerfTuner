from rcc.clients.openrgb.effects.base.abstract_effect import AbstractEffect
from rcc.utils.singleton import singleton
from rcc.utils.openrgb import OpenRGBUtils


@singleton
class SpectrumCycle(AbstractEffect):
    """Spectrum Cycle efect"""

    def __init__(self):
        super().__init__("Spectrum cycle")

    def apply_effect(self):
        offset = 0
        while self._is_running:
            for dev in self._devices:
                colors = [OpenRGBUtils.from_hsv(offset, 1, 1)] * len(dev.leds)
                self._set_colors(dev, colors)
            offset = (offset + 1) % 360
            self._sleep(0.02)  # 20 ms


spectrum_cycle = SpectrumCycle()
