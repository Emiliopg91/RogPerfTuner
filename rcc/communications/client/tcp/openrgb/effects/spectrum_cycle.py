from openrgb.utils import RGBColor

from framework.singleton import singleton
from rcc.communications.client.tcp.openrgb.effects.base.abstract_effect import AbstractEffect


@singleton
class SpectrumCycle(AbstractEffect):
    """Spectrum Cycle efect"""

    def __init__(self):
        super().__init__("Spectrum cycle")

    def apply_effect(self):
        offset = 0
        while self._is_running:
            for dev in self._devices:
                if dev.enabled:
                    colors = [RGBColor.fromHSV(offset, 100, 100)] * len(dev.leds)
                    self._set_colors(dev, colors)
            offset = (offset + 1) % 360
            self._sleep(0.02)  # 20 ms


SPECTRUM_CYCLE_EFFECT = SpectrumCycle()
