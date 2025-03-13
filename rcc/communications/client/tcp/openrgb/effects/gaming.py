from math import floor
import re
from framework.singleton import singleton
from rcc.communications.client.tcp.openrgb.client.utils import DeviceType, RGBColor
from rcc.communications.client.tcp.openrgb.effects.base.abstract_effect import AbstractEffect


@singleton
class GamingEffect(AbstractEffect):
    """Gaming effect"""

    COMMON1_COLOR = RGBColor.fromHEX("1010AA")
    NUMBER_COLOR = RGBColor.fromHEX("#FFFF00")
    WASD_COLOR = RGBColor.fromHEX("#FF0000")

    NUM_KEY_REGEX = re.compile(r"Key: [1|2|3|4]")
    WASD_KEY_REGEX = re.compile(r"Key: [W|A|S|D]")

    def __init__(self):
        super().__init__("Gaming")

    def apply_effect(self):
        while self._is_running:  # pylint: disable=too-many-nested-blocks
            for dev in self._devices:
                colors = [self.COMMON1_COLOR] * len(dev.leds)
                offset = 0
                if dev.type == DeviceType.KEYBOARD:
                    for zone in dev.zones:
                        for idx, led in enumerate(zone.leds):
                            if self.NUM_KEY_REGEX.fullmatch(led.name):
                                colors[offset + idx] = self.NUMBER_COLOR
                            elif self.WASD_KEY_REGEX.fullmatch(led.name):
                                colors[offset + idx] = self.WASD_COLOR
                        offset += len(zone.leds)
                self._set_colors(dev, colors)
            self._sleep(0.3)


GAMING_EFFECT = GamingEffect()
