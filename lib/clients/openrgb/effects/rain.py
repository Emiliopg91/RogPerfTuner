import random
import threading

# pylint: disable=E0611, E0401
from openrgb.utils import RGBColor
from lib.clients.openrgb.effects.base.abstract_effect import AbstractEffect
from lib.utils.singleton import singleton


@singleton
class Rain(AbstractEffect):
    """Rain effect"""

    def __init__(self):
        super().__init__("Rain")
        self.available_colors = [
            RGBColor(0, 0, 0),
            RGBColor(0, 0, 0),
            RGBColor(255, 255, 255),
            RGBColor(255, 0, 0),
            RGBColor(0, 255, 255),
        ]
        self.buffer = []

    def apply_effect(self):
        self.buffer = [None] * len(self.devices)
        threads = []
        for i, _ in self.devices:
            self.buffer[i] = []
            thread = threading.Thread(
                name=f"Rain-dev-{i}",
                target=self._effect_thread,
                args=(i, self.devices[i]),
            )
            threads.append(thread)
            thread.start()

        # Esperar a que todos los hilos terminen
        for thread in threads:
            thread.join()

    def _effect_thread(self, dev_index, dev):
        leds = [RGBColor.fromHEX("#000000")] * len(dev.leds)

        while self.is_running:
            next_t = self._get_next(dev_index, dev)
            leds[next_t["index"]] = next_t["color"]
            self._set_colors(dev, leds)
            naps = 4
            for _ in range(naps):
                if self.is_running:
                    nap_time = 2500 / len(leds) / (0.5 + random.random() * 0.5) / naps
                    self._sleep(nap_time / 1000)

    def _get_next(self, dev_index, dev):
        if len(self.buffer[dev_index]) == 0:
            for i in range(len(dev.leds)):
                self.buffer[dev_index].append(
                    {
                        "index": i,
                        "color": self.available_colors[int(random.random() * len(self.available_colors))],
                    }
                )
            for i in range(len(dev.leds)):
                swap = int(random.random() * len(self.buffer[dev_index]))
                tmp = self.buffer[dev_index][i]
                self.buffer[dev_index][i] = self.buffer[dev_index][swap]
                self.buffer[dev_index][swap] = tmp

        return self.buffer[dev_index].pop(0)


rain = Rain()
