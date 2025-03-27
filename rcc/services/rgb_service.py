import importlib
import os
from typing import Optional
import time


from rcc.communications.client.dbus.linux.power_management_keyboard_brightness_control import (
    KEYBOARD_BRIGHTNESS_CONTROL,
)
from rcc.communications.client.dbus.linux.upower_client import UPOWER_CLIENT
from rcc.communications.client.tcp.openrgb.openrgb_client import OPEN_RGB_CLIENT
from rcc.communications.client.tcp.openrgb.effects.static import STATIC_EFFECT
from rcc.models.rgb_brightness import RgbBrightness
from rcc.models.settings import Effect
from rcc.utils.events import HARDWARE_SERVICE_ON_BATTERY, HARDWARE_SERVICE_ON_USB_CHANGED
from rcc.utils.constants import USER_EFFECTS_FOLDER
from rcc.utils.configuration import CONFIGURATION
from rcc.utils.beans import EVENT_BUS
from framework.logger import Logger
from framework.singleton import singleton


@singleton
class RgbService:
    """Service for mangeing RGB lightning"""

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing RgbService")
        self._logger.add_tab()

        self._effect = STATIC_EFFECT.name
        if CONFIGURATION.open_rgb.last_effect:
            self._effect = CONFIGURATION.open_rgb.last_effect

        self._brightness = RgbBrightness.MAX
        if CONFIGURATION.open_rgb.brightness is not None:
            self._brightness = RgbBrightness(CONFIGURATION.open_rgb.brightness)

        self._color = STATIC_EFFECT.color
        if CONFIGURATION.open_rgb.last_effect and CONFIGURATION.open_rgb.last_effect in CONFIGURATION.open_rgb.effects:
            self._color = CONFIGURATION.open_rgb.effects[CONFIGURATION.open_rgb.last_effect].color

        self.__load_custom_effects()

        self._logger.info("Restoring effect")
        self._logger.add_tab()
        OPEN_RGB_CLIENT.apply_effect(
            self._effect, RgbBrightness.OFF if UPOWER_CLIENT.on_battery else self._brightness, self._color
        )
        self._logger.rem_tab()
        self._logger.rem_tab()

        EVENT_BUS.on(HARDWARE_SERVICE_ON_USB_CHANGED, self.reload)
        EVENT_BUS.on(HARDWARE_SERVICE_ON_BATTERY, self.__on_battery_handler)
        if KEYBOARD_BRIGHTNESS_CONTROL.available:
            KEYBOARD_BRIGHTNESS_CONTROL.on_brightness_change(self.__on_keyboard_brightness_change)

    def __on_battery_handler(self, on_bat: bool, keyboard_value: int = None):
        if KEYBOARD_BRIGHTNESS_CONTROL.available:
            if keyboard_value is None:
                keyboard_value = KEYBOARD_BRIGHTNESS_CONTROL.keyboard_brightness

            if keyboard_value == 0:
                self._apply_aura(self._effect, RgbBrightness.OFF, self._color, True, True)
            elif on_bat:
                OPEN_RGB_CLIENT.apply_effect(self._effect, RgbBrightness.OFF)
            else:
                self.restore_brightness()
        else:
            OPEN_RGB_CLIENT.apply_effect(self._effect, RgbBrightness.OFF if on_bat else self._brightness, self._color)

    def __on_keyboard_brightness_change(self, value: int):
        if value == 0:
            self._apply_aura(self._effect, RgbBrightness.OFF, self._color, True, True)
        else:
            self.__on_battery_handler(UPOWER_CLIENT.on_battery, value)

    def __load_custom_effects(self):
        """Load user custom effect"""
        directory = USER_EFFECTS_FOLDER
        files = [file for file in os.listdir(directory) if file.endswith(".py") and file != "__init__.py"]

        if len(files) > 0:
            self._logger.info(f"Loading {len(files)} custom effects")
            self._logger.add_tab()
            for file in files:
                module_name = file[:-3]
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

    def reload(self) -> None:
        """Reload OpenRGB Server"""
        t0 = time.time()
        self._logger.info("Reloading OpenRGB server")
        self._logger.add_tab()
        OPEN_RGB_CLIENT.stop()
        OPEN_RGB_CLIENT.start()
        self._logger.rem_tab()
        self._apply_aura(self._effect, self._brightness, self._color, True)
        self._logger.info(f"Reloaded after {(time.time() - t0):.2f} seconds")

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

        if effect in CONFIGURATION.open_rgb.effects:
            return CONFIGURATION.open_rgb.effects[effect].color

        from_cli = OPEN_RGB_CLIENT.get_color(effect)

        if from_cli is not None:
            return from_cli

        return "#000000"

    def apply_effect(self, effect: str, temporal=False) -> None:
        """Apply effect"""
        color = None
        if OPEN_RGB_CLIENT.supports_color(effect):
            if effect in CONFIGURATION.open_rgb.effects:
                color = CONFIGURATION.open_rgb.effects[effect].color
            else:
                color = OPEN_RGB_CLIENT.get_color(effect)

        self._apply_aura(effect, self.brightness, color, temporal=temporal)

    def apply_brightness(self, brightness: RgbBrightness) -> None:
        """Apply brightness"""
        self._apply_aura(self._effect, brightness, self._color)

    def apply_color(self, color: str) -> None:
        """Apply color"""
        self._apply_aura(self._effect, self._brightness, color)

    def restore_effect(self):
        """Restore last not temporal effect"""
        self.apply_effect(CONFIGURATION.open_rgb.last_effect)

    def restore_brightness(self):
        """Restore last not temporal effect"""
        self.apply_brightness(RgbBrightness(CONFIGURATION.open_rgb.brightness))

    def _apply_aura(  # pylint: disable=too-many-arguments,too-many-positional-arguments
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
            EVENT_BUS.emit(
                "OpenRgbService.aura_changed",
                {effect: effect, brightness: brightness, color: color},
            )

            if not temporal:
                CONFIGURATION.open_rgb.last_effect = effect
                CONFIGURATION.open_rgb.brightness = brightness.value

                if supports_color:
                    CONFIGURATION.open_rgb.effects[effect] = Effect(color)
                CONFIGURATION.save_config()

            self._logger.rem_tab()

    def get_next_effect(self) -> str:
        """Get next effect name"""
        effects = self.get_available_effects()
        index = (effects.index(self._effect) + 1) % len(effects)
        return effects[index]


RGB_SERVICE = RgbService()
