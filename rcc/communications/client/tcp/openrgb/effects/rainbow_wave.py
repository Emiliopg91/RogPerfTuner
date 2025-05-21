import math

from openrgb.utils import RGBColor, ZoneType

from framework.singleton import singleton


from rcc.communications.client.tcp.openrgb.effects.base.abstract_effect import AbstractEffect


@singleton
class RainbowWave(AbstractEffect):
    """Rainbow wave effect"""

    def __init__(self):
        super().__init__("Rainbow wave")

    def apply_effect(self):
        longest_zone = max(set(max(set(zone.mat_width or len(zone.leds) for zone in el.zones)) for el in self._devices))

        rainbow = [0] * (3 * longest_zone)
        inc = 360 / len(rainbow)
        for idx in range(len(rainbow) - 1, -1, -1):
            rainbow[idx] = (len(rainbow) - idx) * inc

        while self._is_running:  # pylint: disable=too-many-nested-blocks
            for dev in self._devices:
                if dev.enabled:
                    colors = [RGBColor(0, 0, 0)] * len(dev.leds)
                    offset = 0
                    for zone in dev.zones:
                        if zone.type == ZoneType.MATRIX:
                            for r in range(zone.mat_height):
                                for c in range(zone.mat_width):
                                    if zone.matrix_map[r][c] is not None:
                                        rainbow_index = round((0.85 * len(rainbow)) * (c / zone.mat_width))
                                        colors[offset + zone.matrix_map[r][c]] = RGBColor.fromHSV(
                                            rainbow[rainbow_index], 100, 100
                                        )
                        else:
                            for l in range(len(zone.leds)):
                                rainbow_index = math.floor((0.85 * len(rainbow)) * (l / len(zone.leds)))
                                colors[offset + l] = RGBColor.fromHSV(rainbow[rainbow_index], 100, 100)
                        offset += len(zone.leds)
                    self._set_colors(dev, colors)

            self._sleep(4 / len(rainbow))

            rainbow.insert(0, (rainbow[0] + inc) % 360)
            rainbow.pop()


RAINBOW_WAVE_EFFECT = RainbowWave()
