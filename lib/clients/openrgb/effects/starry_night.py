from .base.abstract_effect import AbstractEffect
from ..sdk.utils import RGBColor
from ....utils.singleton import singleton

import random
import threading


@singleton
class StarryNight(AbstractEffect):
    def __init__(self):
        super().__init__("Starry night")

    def get_random(self):
        hue = random.randint(0, 359)
        return RGBColor.fromHSV(hue, 1, 2)

    def apply_effect(self):
        self.max_steps = 30
        self.threads = []

        for device in self.devices:
            thread = threading.Thread(
                name=f"StarryNight-dev-{device.id}",
                target=self.device_effect,
                args=(device,),
            )
            self.threads.append(thread)
            thread.start()

        for thread in self.threads:
            thread.join()

    def device_effect(self, device):
        leds = [RGBColor.fromHEX("#000000")] * len(device.leds)
        steps = [0] * len(device.leds)

        while self.is_running:
            new_colors = [None] * len(device.leds)

            for i in range(len(leds)):
                steps[i] = max(0, steps[i] - 1)
                new_colors[i] = leds[i].dim(steps[i] / self.max_steps)

            can_turn_on = sum(1 for i in steps if i > 0) / len(steps) < 0.1

            if can_turn_on:
                led_on = -1
                while led_on < 0 or steps[led_on] > 0:
                    led_on = random.randint(0, len(leds) - 1)
                steps[led_on] = 15 + random.randint(0, 15)
                new_colors[led_on] = leds[led_on] = self.get_random().dim(
                    steps[led_on] / self.max_steps
                )

            self.set_colors(device, new_colors)
            self.sleep(random.randint(0, 150) / 1000)


starry_night = StarryNight()
