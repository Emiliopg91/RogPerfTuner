import time

from abc import ABC, abstractmethod
from threading import Lock, Thread
from typing import List, Optional

# pylint: disable=E0611, E0401
from openrgb.orgb import Device
from openrgb.utils import RGBColor

from lib.models.rgb_brightness import RgbBrightness
from lib.utils.logger import Logger
from lib.utils.openrgb import OpenRGBUtils


class AbstractEffect(ABC):
    """Base class for effects"""

    def __init__(self, name: str, default_color: str = None):
        self.is_running = False
        self.brightness = 0
        self._supports_color = default_color is not None
        self._name = name
        self._color = RGBColor.fromHEX(default_color) if default_color is not None else None
        self.logger = Logger()
        self.mutex = Lock()
        self.devices: List[Device] = []
        self.thread = None

    def start(self, devices: List[Device], brightness: RgbBrightness, color: RGBColor):
        """Start effect"""
        if self.is_running:
            self.stop()

        self.thread = None
        self.is_running = True

        if brightness == RgbBrightness.OFF:
            self.logger.info("Turning off RGB")
            for dev in devices:
                dev.set_colors([RGBColor.fromHEX("#000000")] * len(dev.leds), True)
            return

        self._color = color
        self.devices = devices
        self.logger.info(
            f"Starting effect with {brightness.name.lower()} brightness"
            f"{' and ' + OpenRGBUtils.to_hex(color) + ' color' if self._supports_color else ''}"
        )

        # Map brightness levels
        brightness_mapping = {
            RgbBrightness.LOW: 0.25,
            RgbBrightness.MEDIUM: 0.5,
            RgbBrightness.HIGH: 0.75,
            RgbBrightness.MAX: 1,
        }
        self.brightness = brightness_mapping.get(brightness, 0)

        self.is_running = True

        self.thread = Thread(name=self.__class__.__name__, target=self._thread_main)
        self.thread.start()

    def stop(self):
        """Stop effect"""
        if self.is_running:
            self.logger.info("Stopping effect")
            self.is_running = False

            if self.thread is not None:
                self.thread.join()

    def _sleep(self, ms: int):
        if self.is_running:
            time.sleep(ms)

    def _set_colors(self, dev: Device, colors: List[RGBColor]):
        if self.is_running:
            with self.mutex:
                if self.is_running:
                    dimmed_colors = [OpenRGBUtils.dim(color, self.brightness) for color in colors]
                    dev.set_colors(dimmed_colors, True)

    @property
    def supports_color(self) -> bool:
        """Check if effect supports color"""
        return self._supports_color

    @property
    def name(self) -> str:
        """Get effect name"""
        return self._name

    @property
    def color(self) -> Optional[str]:
        """Get hex color or none"""
        return OpenRGBUtils.to_hex(self._color) if self._supports_color else None

    def _thread_main(self):
        self.apply_effect()
        self.logger.info("Effect finished")
        self.thread = None

    @abstractmethod
    def apply_effect(self):
        """Apply effect"""
