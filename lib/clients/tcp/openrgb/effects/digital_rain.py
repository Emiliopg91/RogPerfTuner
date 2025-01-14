# pylint: disable=E0611,E0401

import math
import random
import threading

import psutil
from openrgb.orgb import Device, Zone
from openrgb.utils import RGBColor, ZoneType

from lib.clients.tcp.openrgb.effects.base.abstract_effect import AbstractEffect
from lib.utils.openrgb import OpenRGBUtils
from lib.utils.singleton import singleton


@singleton
class DigitalRain(AbstractEffect):
    """Digital rain effect"""

    def __init__(self):
        super().__init__("Digital rain", "#00FF00")
        self._decrement = 1.4
        self._max_count = int(pow(self._decrement, 15))

    def _initialize_matrix(self, zone_status: list[list[int]], zone: Zone):
        for i in range(zone.mat_height):
            zone_status.append([])
            for _ in range(zone.mat_width):
                zone_status[i].append(0)

    def _initialize_linear(self, zone_status: list[int], zone: Zone):
        for _ in range(len(zone.leds)):
            zone_status.append(0)

    def _decrement_matrix(self, zone_status: list[list[int]], zone: Zone):
        for r in range(zone.mat_height - 1, -1, -1):  # pylint: disable=R1702
            for c in range(zone.mat_width):
                if r == 0:
                    if zone_status[r][c] == self._max_count:
                        zone_status[r][c] = math.floor(zone_status[r][c] / pow(1.4, random.random() * 5))
                    else:
                        if zone_status[r][c] > 0:
                            zone_status[r][c] = math.floor(zone_status[r][c] / self._decrement)
                            if zone_status[r][c] == 0:
                                zone_status[r][c] = 0
                        elif zone_status[r][c] < 0:
                            zone_status[r][c] += 1
                else:
                    zone_status[r][c] = zone_status[r - 1][c]

    def _decrement_linear(self, zone_status: list[int], zone: Zone):
        for r in range(len(zone.leds) - 1, -1, -1):
            if r == 0:
                if zone_status[r] > 0:
                    zone_status[r] = math.floor(zone_status[r] / self._decrement)
            else:
                zone_status[r] = zone_status[r - 1]

    def _to_color_matrix(self, zone_status: list[list[int]], zone: Zone):
        colors = [RGBColor(0, 0, 0) for _ in range(len(zone.leds))]
        for r in range(zone.mat_height):  # pylint: disable=R1702
            for c in range(zone.mat_width):
                if zone.matrix_map[r][c] is not None:
                    if zone_status[r][c] >= 0:
                        if zone_status[r][c] == self._max_count:
                            colors[zone.matrix_map[r][c]] = RGBColor(255, 255, 255)
                        else:
                            colors[zone.matrix_map[r][c]] = OpenRGBUtils.dim(
                                self._color, zone_status[r][c] / self._max_count
                            )
                    else:
                        colors[zone.matrix_map[r][c]] = RGBColor(0, 0, 0)
        return colors

    def _to_color_linear(self, zone_status: list[int], zone: Zone):
        colors = [RGBColor(0, 0, 0) for _ in range(len(zone.leds))]
        for r in range(len(zone.leds)):
            if zone_status[r] == self._max_count:
                colors[r] = RGBColor(255, 255, 255)
            else:
                colors[r] = OpenRGBUtils.dim(self._color, zone_status[r] / self._max_count)
        return colors

    def _is_matrix_column_off(self, zone_status: list[list[int]], height: int, column: int):
        for r in range(height):
            if zone_status[r][column] != 0:
                return False
        return True

    def _get_next_matrix(self, zone_status: list[list[int]], zone: Zone):  # pylint: disable=R0912
        free_cols: list[int] = []
        for c in range(zone.mat_width):
            if self._is_matrix_column_off(zone_status, zone.mat_height, c):
                free_cols.append(c)

        if len(free_cols) > 0:
            cpu = max(1, psutil.cpu_percent()) / 100
            allowed = max(1, math.ceil(zone.mat_width * cpu))

            if allowed > zone.mat_width - len(free_cols):
                next_col = free_cols[random.randint(0, len(free_cols) - 1)]
                zone_status[0][next_col] = self._max_count

    def _get_next_linear(self, zone_status: list[int], zone: Zone):
        free_cols: list[int] = []
        for c in range(len(zone.leds)):
            if zone_status[c] == 0:
                free_cols.append(c)

        if len(free_cols) > 0:
            next_led = free_cols[random.randint(0, len(free_cols) - 1)]
            zone_status[next_led] = self._max_count

    def apply_effect(self):
        zone_status = []
        colors = []
        threads = []

        def device_thread(dev: Device, dev_id):
            colors.append([RGBColor(0, 0, 0) for _ in dev.colors])
            self._set_colors(dev, colors[dev_id])

            for iz, zone in enumerate(dev.zones):
                zone_status[dev_id].append([])

                if zone.type == ZoneType.MATRIX:
                    self._initialize_matrix(zone_status[dev_id][iz], zone)
                elif zone.type in (ZoneType.LINEAR, ZoneType.SINGLE):
                    self._initialize_linear(zone_status[dev_id][iz], zone)

            self._sleep(random.randint(0, 100) / 1000)

            iter_count = 0
            while self._is_running:
                offset = 0
                final_colors = [RGBColor(0, 0, 0) for _ in dev.colors]

                for iz, zone in enumerate(dev.zones):
                    zone_colors = None

                    if zone.type == ZoneType.MATRIX:
                        self._decrement_matrix(zone_status[dev_id][iz], zone)
                        self._get_next_matrix(zone_status[dev_id][iz], zone)
                        zone_colors = self._to_color_matrix(zone_status[dev_id][iz], zone)
                    elif zone.type in (ZoneType.LINEAR, ZoneType.SINGLE):
                        self._decrement_linear(zone_status[dev_id][iz], zone)
                        self._get_next_linear(zone_status[dev_id][iz], zone)
                        zone_colors = self._to_color_linear(zone_status[dev_id][iz], zone)

                    for i, color in enumerate(zone_colors):
                        final_colors[offset + i] = color
                    offset += len(zone.leds)

                self._set_colors(dev, final_colors)
                self._sleep(0.06)
                iter_count = (iter_count + 1) % 100

        for dev_id, dev in enumerate(self._devices):
            zone_status.append([])
            thread = threading.Thread(
                name=f"DigitalRain-dev-{dev_id}",
                target=device_thread,
                args=(dev, dev_id),
            )
            threads.append(thread)
            thread.start()

        for thread in threads:
            thread.join()


digital_rain = DigitalRain()
