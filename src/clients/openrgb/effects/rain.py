from .base.abstract_effect import AbstractEffect
from ..sdk.utils import RGBColor
from ....utils.singleton import singleton

import random
import threading
import time


@singleton
class Rain(AbstractEffect):
    def __init__(self):
        super().__init__("Rain")
        self.available_colors = [
            RGBColor(0, 0, 0),
            RGBColor(0, 0, 0),
            RGBColor(255, 255, 255),
            RGBColor(255, 0, 0),
            RGBColor(0, 255, 255),
        ]

    def apply_effect(self):
        self.buffer = [None] * len(self.devices)
        threads = []
        for i in range(len(self.devices)):
            self.buffer[i] = []
            thread = threading.Thread(
                target=self.effect_thread, args=(i, self.devices[i])
            )
            threads.append(thread)
            thread.start()

        # Esperar a que todos los hilos terminen
        for thread in threads:
            thread.join()

    def effect_thread(self, dev_index, dev):
        leds = [RGBColor.fromHEX("#000000")] * len(dev.leds)

        while self.is_running:
            next = self.get_next(dev_index, dev)
            leds[next["index"]] = next["color"]
            self.set_colors(dev, leds)
            naps = 4
            for nap in range(naps):
                if self.is_running:
                    nap_time = 2500 / len(leds) / (0.5 + random.random() * 0.5) / naps
                    self.sleep(nap_time / 1000)

    def get_next(self, dev_index, dev):
        if len(self.buffer[dev_index]) == 0:
            for i in range(len(dev.leds)):
                self.buffer[dev_index].append(
                    {
                        "index": i,
                        "color": self.available_colors[
                            int(random.random() * len(self.available_colors))
                        ],
                    }
                )
            for i in range(len(dev.leds)):
                swap = int(random.random() * len(self.buffer[dev_index]))
                tmp = self.buffer[dev_index][i]
                self.buffer[dev_index][i] = self.buffer[dev_index][swap]
                self.buffer[dev_index][swap] = tmp

        return self.buffer[dev_index].pop(0)


rain = Rain()
