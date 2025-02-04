import importlib
import os
from threading import Lock, Thread
from typing import Optional
import subprocess
import time

import pyudev

from rcc.communications.client.tcp.openrgb.openrgb_client import OPEN_RGB_CLIENT
from rcc.communications.client.tcp.openrgb.effects.static import STATIC_EFFECT
from rcc.models.rgb_brightness import RgbBrightness
from rcc.models.settings import Effect
from rcc.models.usb_identifier import UsbIdentifier
from rcc.utils.constants import USER_EFFECTS_FOLDER
from rcc.utils.configuration import configuration
from rcc.utils.beans import event_bus
from framework.logger import Logger
from framework.singleton import singleton


@singleton
class OpenRgbService:
    """Service for mangeing RGB lightning"""

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing OpenRgbService")
        self._logger.add_tab()

        self._effect = STATIC_EFFECT.name
        if configuration.open_rgb.last_effect:
            self._effect = configuration.open_rgb.last_effect

        self._brightness = RgbBrightness.MAX
        if configuration.open_rgb.brightness is not None:
            self._brightness = RgbBrightness(configuration.open_rgb.brightness)

        self._color = STATIC_EFFECT.color
        if configuration.open_rgb.last_effect and configuration.open_rgb.last_effect in configuration.open_rgb.effects:
            self._color = configuration.open_rgb.effects[configuration.open_rgb.last_effect].color

        self._connected_usb: list[UsbIdentifier] = []
        self._usb_mutex = Lock()
        thread = Thread(name="UsbChecker", target=self.monitor_for_usb)
        thread.start()

        self.__load_custom_effects()

        self._logger.info("Restoring effect")
        self._logger.add_tab()
        OPEN_RGB_CLIENT.apply_effect(self._effect, self._brightness, self._color)
        self._logger.rem_tab()
        self._logger.rem_tab()

    def __load_custom_effects(self):
        """Load user custom effect"""
        directory = USER_EFFECTS_FOLDER
        files = [file for file in os.listdir(directory) if file.endswith(".py") and file != "__init__.py"]

        if len(files) > 0:
            self._logger.info(f"Loading {len(files)} custom effects")
            self._logger.add_tab()
            for file in files:
                module_name = file[:-3]  # Quitar ".py"
                module_path = os.path.join(directory, file)
                self._logger.debug(f"Loading effect file {module_path}")
                spec = importlib.util.spec_from_file_location(module_name, module_path)
                module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(module)

                if hasattr(module, "instance"):
                    instance = getattr(module, "instance")
                    self._logger.info(f"Loaded effect {instance.name}")
                    OPEN_RGB_CLIENT.append_custom_effect(instance)
            self._logger.rem_tab()

    @property
    def brightness(self) -> RgbBrightness:
        """Getter for brightness"""
        return self._brightness

    @property
    def effect(self) -> str:
        """Getter for effect"""
        return self._effect

    @property
    def color(self) -> str:
        """Getter for color"""
        return self._color

    def bounced_reload(self) -> None:
        """Reload OpenRGB Server"""
        self._usb_mutex.acquire(True)  # pylint: disable=R1732
        try:
            t0 = time.time()
            self._logger.info("Reloading OpenRGB server")
            self._logger.add_tab()
            OPEN_RGB_CLIENT.stop()
            OPEN_RGB_CLIENT.start()
            self._logger.rem_tab()
            self._apply_aura(self._effect, self._brightness, self._color, True)
            self._logger.info(f"Reloaded after {(time.time() - t0):.2f} seconds")
        finally:
            self._usb_mutex.release()

    def monitor_for_usb(self) -> None:  # pylint: disable=R0914, R0912
        """Monitor for usb devices changes"""
        monitor = pyudev.Monitor.from_netlink(pyudev.Context())
        monitor.filter_by("usb")

        lsusb_output = subprocess.check_output(["lsusb"]).decode("utf-8").strip()
        current_usb = []
        for line in lsusb_output.split("\n"):
            columns = line.strip().split(" ")

            id_vendor, id_product = columns[5].split(":")
            name = " ".join(columns[6:])

            usb_dev = UsbIdentifier(id_vendor, id_product, name)

            for cd in OPEN_RGB_CLIENT.compatible_devices:
                if cd.id_vendor == usb_dev.id_vendor and cd.id_product == usb_dev.id_product:
                    self._connected_usb.append(cd)

        for action, _ in monitor:  # pylint: disable=R1702
            if action in ["add", "remove"]:
                self._usb_mutex.acquire(True)  # pylint: disable=R1732
                try:
                    lsusb_output = subprocess.check_output(["lsusb"]).decode("utf-8").strip()

                    current_usb = []
                    for line in lsusb_output.split("\n"):
                        columns = line.strip().split(" ")

                        id_vendor, id_product = columns[5].split(":")
                        name = " ".join(columns[6:])

                        usb_dev = UsbIdentifier(id_vendor, id_product, name)

                        if any(
                            cd.id_vendor == usb_dev.id_vendor and cd.id_product == usb_dev.id_product
                            for cd in OPEN_RGB_CLIENT.compatible_devices
                        ):
                            current_usb.append(usb_dev)

                    added = []
                    for dev1 in current_usb:
                        found = False
                        for dev2 in self._connected_usb:
                            if not found and dev1.id_vendor == dev2.id_vendor and dev1.id_product == dev2.id_product:
                                found = True
                        if not found:
                            added.append(dev1)

                    removed = []
                    for dev1 in self._connected_usb:
                        found = False
                        for dev2 in current_usb:
                            if not found and dev1.id_vendor == dev2.id_vendor and dev1.id_product == dev2.id_product:
                                found = True
                        if not found:
                            removed.append(dev1)

                    if len(removed) > 0:
                        self._logger.info("Removed compatible device(s):")
                        self._logger.add_tab()
                        for item in removed:
                            self._logger.info(OPEN_RGB_CLIENT.get_device_name(item.id_vendor, item.id_product))
                            OPEN_RGB_CLIENT.disable_device(item.name)
                        self._logger.rem_tab()

                    if len(added) > 0:
                        self._logger.info("Connected compatible device(s):")
                        self._logger.add_tab()
                        for item in added:
                            self._logger.info(OPEN_RGB_CLIENT.get_device_name(item.id_vendor, item.id_product))
                        self._logger.rem_tab()

                        self._usb_mutex.release()
                        self.bounced_reload()
                    else:
                        self._usb_mutex.release()

                    self._connected_usb = current_usb
                except Exception:
                    self._usb_mutex.release()

    def get_available_effects(self) -> list[str]:
        """Get all available effects"""
        return OPEN_RGB_CLIENT.get_available_effects()

    def supports_color(self, effect=None) -> bool:
        """Check if effect supports color"""
        if effect is None:
            effect = self._effect
        return OPEN_RGB_CLIENT.supports_color(effect)

    def get_color(self, effect=None) -> str | None:
        """Get color for effect"""
        if effect is None:
            effect = self._effect

        if effect in configuration.open_rgb.effects:
            return configuration.open_rgb.effects[effect].color

        from_cli = OPEN_RGB_CLIENT.get_color(effect)

        if from_cli is not None:
            return from_cli

        return "#000000"

    def apply_effect(self, effect: str) -> None:
        """Apply effect"""
        color = None
        if OPEN_RGB_CLIENT.supports_color(effect):
            if effect in configuration.open_rgb.effects:
                color = configuration.open_rgb.effects[effect].color
            else:
                color = OPEN_RGB_CLIENT.get_color(effect)

        self._apply_aura(effect, self.brightness, color)

    def apply_brightness(self, brightness: RgbBrightness) -> None:
        """Apply brightness"""
        self._apply_aura(self._effect, brightness, self._color)

    def apply_color(self, color: str) -> None:
        """Apply color"""
        self._apply_aura(self._effect, self._brightness, color)

    def _apply_aura(  # pylint: disable=R0913,R0917
        self,
        effect: str,
        brightness: RgbBrightness,
        color: Optional[str] = None,
        force: bool = False,
        temporal: bool = False,
    ) -> None:
        color = color if OPEN_RGB_CLIENT.supports_color(effect) else None
        if (self._effect != effect or self._brightness != brightness or self._color != color) or force:
            self._logger.info("Applying effect")
            self._logger.add_tab()
            supports_color = OPEN_RGB_CLIENT.apply_effect(effect, brightness, color)

            self._effect = effect
            self._brightness = brightness
            self._color = color
            event_bus.emit(
                "OpenRgbService.aura_changed",
                {effect: effect, brightness: brightness, color: color},
            )

            if not temporal:
                configuration.open_rgb.last_effect = effect
                configuration.open_rgb.brightness = brightness.value

                if supports_color:
                    configuration.open_rgb.effects[effect] = Effect(color)
                configuration.save_config()

            self._logger.rem_tab()

    def get_next_effect(self) -> str:
        """Get next effect name"""
        effects = self.get_available_effects()
        index = (effects.index(self._effect) + 1) % len(effects)
        return effects[index]


OPEN_RGB_SERVICE = OpenRgbService()
