from typing import Callable
from rcc.communications.client.dbus.abstract_dbus_client import AbstractDbusClient
from framework.singleton import singleton


@singleton
class PowerManagementKeyboardBrightnessControl(AbstractDbusClient):
    """Dbus upower client"""

    def __init__(self):
        super().__init__(
            False,
            "org.kde.Solid.PowerManagement",
            "/org/kde/Solid/PowerManagement/Actions/KeyboardBrightnessControl",
            "org.kde.Solid.PowerManagement.Actions.KeyboardBrightnessControl",
            required=False,
        )

    @property
    def keyboard_brightness_changed(self) -> bool:
        """keyboardBrightnessChanged"""
        return self._get_property("keyboardBrightnessChanged")

    def on_brightness_change(self, callback: Callable[[int], None]):
        """Subscribe keyboard brightness change"""
        self.on_signal("keyboardBrightnessChanged", callback)


KEYBOARD_BRIGHTNESS_CONTROL = PowerManagementKeyboardBrightnessControl()
