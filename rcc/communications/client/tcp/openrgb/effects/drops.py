import random
import threading

from framework.singleton import singleton
from rcc.communications.client.tcp.openrgb.client.utils import RGBColor
from rcc.communications.client.tcp.openrgb.effects.base.abstract_effect import AbstractEffect


@singleton
class Drops(AbstractEffect):
    """Drops effect"""

    def __init__(self):
        super().__init__("Drops")
        self._available_colors = [
            RGBColor(0, 0, 0),
            RGBColor(0, 0, 0),
            RGBColor(255, 255, 255),
            RGBColor(255, 0, 0),
            RGBColor(0, 255, 255),
        ]
        self._buffer = []

    def apply_effect(self):
        self._buffer = [None] * len(self._devices)
        threads = []
        for i in range(len(self._devices)):  # pylint: disable=consider-using-enumerate
            self._buffer[i] = []
            thread = threading.Thread(
                name=f"Drops-dev-{i}",
                target=self._effect_thread,
                args=(i, self._devices[i]),
            )
            threads.append(thread)
            thread.start()

        # Esperar a que todos los hilos terminen
        for thread in threads:
            thread.join()

    def _effect_thread(self, dev_index, dev):
        leds = [RGBColor.fromHEX("#000000")] * len(dev.leds)

        while self._is_running and dev.enabled:
            next_t = self._get_next(dev_index, dev)
            leds[next_t["index"]] = next_t["color"]
            self._set_colors(dev, leds)
            naps = 4
            for _ in range(naps):
                if self._is_running:
                    nap_time = 2500 / len(leds) / (random.randint(6, 10) / 10) / naps
                    self._sleep(nap_time / 1000)

    def _get_next(self, dev_index, dev):
        if len(self._buffer[dev_index]) == 0:
            for i in range(len(dev.leds)):
                self._buffer[dev_index].append(
                    {
                        "index": i,
                        "color": self._available_colors[random.randint(0, len(self._available_colors) - 1)],
                    }
                )

            random.shuffle(self._buffer[dev_index])

        return self._buffer[dev_index].pop(0)


DROPS_EFFECT = Drops()
