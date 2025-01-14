import random
import threading

# pylint: disable=E0611, E0401
from openrgb.utils import RGBColor

from lib.clients.tcp.openrgb.effects.base.abstract_effect import AbstractEffect
from lib.utils.singleton import singleton
from lib.utils.openrgb import OpenRGBUtils


@singleton
class StarryNight(AbstractEffect):
    """Starry night effect"""

    def __init__(self):
        super().__init__("Starry night")
        self._max_steps = 30
        self._threads = []

    def _get_random(self):
        hue = random.randint(0, 359)
        return OpenRGBUtils.from_hsv(hue, 1, 2)

    def apply_effect(self):
        self._threads = []

        for device in self._devices:
            thread = threading.Thread(
                name=f"StarryNight-dev-{device.id}",
                target=self._device_effect,
                args=(device,),
            )
            self._threads.append(thread)
            thread.start()

        for thread in self._threads:
            thread.join()

    def _device_effect(self, device):
        leds = [RGBColor.fromHEX("#000000")] * len(device.leds)
        steps = [0] * len(device.leds)

        while self._is_running:
            new_colors = [None] * len(device.leds)

            for i in range(len(leds)):  # pylint: disable=C0200
                steps[i] = max(0, steps[i] - 1)
                new_colors[i] = OpenRGBUtils.dim(leds[i], steps[i] / self._max_steps)

            can_turn_on = sum(1 for i in steps if i > 0) / len(steps) < 0.1

            if can_turn_on:
                led_on = -1
                while led_on < 0 or steps[led_on] > 0:
                    led_on = random.randint(0, len(leds) - 1)
                steps[led_on] = 15 + random.randint(0, 15)
                new_colors[led_on] = leds[led_on] = OpenRGBUtils.dim(
                    self._get_random(), steps[led_on] / self._max_steps
                )

            self._set_colors(device, new_colors)
            self._sleep(random.randint(0, 150) / 1000)


starry_night = StarryNight()
