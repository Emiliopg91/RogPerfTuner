import re
from framework.singleton import singleton
from rcc.communications.client.tcp.openrgb.client.utils import DeviceType, RGBColor
from rcc.communications.client.tcp.openrgb.effects.base.abstract_effect import AbstractEffect


@singleton
class GamingEffect(AbstractEffect):
    """Gaming effect"""

    MAIN_COLOR = RGBColor.fromHEX("#0000FF")
    COLOR_MATCHING: list[re.Pattern, RGBColor] = {
        re.compile(r"Key: [1|2|3|4]"): RGBColor.fromHEX("#FFFF00"),
        re.compile(r"Key: [W|A|S|D]"): RGBColor.fromHEX("#FF0000"),
        re.compile(r"Key: Media Volume [-+]|Key: Media Mute"): RGBColor.fromHEX("#00FFFF"),
        re.compile(r"Key: Fan|Key: ROG"): RGBColor.fromHEX("#FF4000"),
    }

    def __init__(self):
        super().__init__("Gaming")

    def apply_effect(self):
        while self._is_running:  # pylint: disable=too-many-nested-blocks
            for dev in self._devices:
                colors = [self.MAIN_COLOR] * len(dev.leds)
                offset = 0
                if dev.type == DeviceType.KEYBOARD:
                    for zone in dev.zones:
                        for idx, led in enumerate(zone.leds):
                            for pattern, color in self.COLOR_MATCHING.items():
                                if pattern.fullmatch(led.name):
                                    colors[offset + idx] = color
                                    break
                        offset += len(zone.leds)
                self._set_colors(dev, colors)

            self._sleep(1)


GAMING_EFFECT = GamingEffect()
