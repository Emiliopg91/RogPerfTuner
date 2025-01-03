import os
import signal
import subprocess

# pylint: disable=E0611, E0401
from PyQt5.QtWidgets import QSystemTrayIcon, QMenu, QAction, QActionGroup, QColorDialog
from PyQt5.QtGui import QIcon, QColor

from lib.gui.main_window import main_window

from lib.models.thermal_throttle_profile import ThermalThrottleProfile
from lib.models.rgb_brightness import RgbBrightness
from lib.models.battery_threshold import BatteryThreshold
from lib.services.openrgb_service import open_rgb_service
from lib.services.platform_service import platform_service
from lib.utils.constants import icons_path, log_file
from lib.utils.event_bus import event_bus
from lib.utils.logger import Logger
from lib.utils.singleton import singleton
from lib.utils.translator import translator


@singleton
class TrayIcon:  # pylint: disable=R0902
    """Tray icon class"""

    def __init__(self):
        self.logger = Logger()

        icon = QIcon.fromTheme(os.path.join(icons_path, "icon-45x45.png"))

        self.tray = QSystemTrayIcon()
        self.tray.setIcon(icon)

        # Create the menu
        self.menu = QMenu()

        # Add "Battery" option (disabled)
        self.battery_action = QAction(translator.translate("battery"))
        self.battery_action.setEnabled(False)
        self.menu.addAction(self.battery_action)

        # Add "Umbral" submenu
        self.umbral_menu = QMenu("    " + translator.translate("charge.threshold"))
        self.threshold_group = QActionGroup(self.umbral_menu)
        self.threshold_group.setExclusive(True)
        self.threshold_actions: dict[BatteryThreshold, QAction] = {}
        for threshold in BatteryThreshold:
            action = QAction(f"{threshold.value}%", checkable=True)
            action.setActionGroup(self.threshold_group)
            action.setChecked(threshold == platform_service.battery_charge_limit)
            action.triggered.connect(lambda _, t=threshold: self.on_threshold_selected(t))
            self.threshold_actions[threshold] = action
            self.umbral_menu.addAction(action)
        self.menu.addMenu(self.umbral_menu)

        self.menu.addSeparator()

        # Add "AuraSync" option (disabled)
        self.aura_section = QAction("AuraSync")
        self.aura_section.setEnabled(False)
        self.menu.addAction(self.aura_section)

        # Add "Effect" submenu
        self.effect_menu = QMenu("    " + translator.translate("effect"))
        self.effect_group = QActionGroup(self.effect_menu)
        self.effect_group.setExclusive(True)
        self.effect_actions: dict[str, QAction] = {}
        for effect in open_rgb_service.get_available_effects():
            action = QAction(effect, checkable=True)
            action.setActionGroup(self.effect_group)
            action.setChecked(effect == open_rgb_service.effect)
            action.triggered.connect(lambda _, e=effect: self.on_effect_selected(e))
            self.effect_actions[effect] = action
            self.effect_menu.addAction(action)
        self.menu.addMenu(self.effect_menu)

        # Add "Brightness" submenu
        self.brightness_menu = QMenu("    " + translator.translate("brightness"))
        self.brightness_group = QActionGroup(self.brightness_menu)
        self.brightness_group.setExclusive(True)
        self.brightness_actions: dict[str, RgbBrightness] = {}
        for brightness in RgbBrightness:
            action = QAction(
                translator.translate(f"label.brightness.{brightness.name}"),
                checkable=True,
            )
            action.setActionGroup(self.brightness_group)
            action.setChecked(brightness == open_rgb_service.brightness)
            action.triggered.connect(lambda _, b=brightness: self.on_brightness_selected(b))
            self.brightness_actions[brightness] = action
            self.brightness_menu.addAction(action)
        self.menu.addMenu(self.brightness_menu)

        # Add "Color" submenu
        self.color_menu = QMenu("    " + translator.translate("color"))
        self.color_menu.setEnabled(open_rgb_service.supports_color())

        self.color_action = QAction(open_rgb_service.get_color())
        self.color_menu.addAction(self.color_action)

        """
        self.colorpicker_action = QAction(translator.translate("select.color"))
        self.colorpicker_action.triggered.connect(self.pick_color)
        self.color_menu.addAction(self.colorpicker_action)
        self.menu.addMenu(self.color_menu)
        """

        self.menu.addSeparator()

        # Add "Performance" option (disabled)
        self.performance_section = QAction(translator.translate("performance"))
        self.performance_section.setEnabled(False)
        self.menu.addAction(self.performance_section)

        self.profile_menu = QMenu("    " + translator.translate("profile"))
        self.profile_group = QActionGroup(self.profile_menu)
        self.profile_group.setExclusive(True)
        self.performance_actions: dict[ThermalThrottleProfile, QAction] = {}
        for profile in ThermalThrottleProfile:
            action = QAction(translator.translate(f"label.profile.{profile.name}"), checkable=True)
            action.setActionGroup(self.profile_group)
            action.setChecked(profile == platform_service.thermal_throttle_profile)
            action.triggered.connect(lambda _, p=profile: self.on_profile_selected(p))
            self.performance_actions[profile] = action
            self.profile_menu.addAction(action)
        self.menu.addMenu(self.profile_menu)

        self.menu.addSeparator()

        # Add "Open" option
        self.open_logs_action = QAction(translator.translate("open.logs"))
        self.open_logs_action.triggered.connect(self.on_open_logs)
        self.menu.addAction(self.open_logs_action)

        self.menu.addSeparator()

        # Add "Open" option
        self.open_action = QAction(translator.translate("open.ui"))
        self.open_action.triggered.connect(self.on_open)
        self.menu.addAction(self.open_action)

        self.menu.addSeparator()

        # Add "Quit" option
        self.quit_action = QAction(translator.translate("close"))
        self.quit_action.triggered.connect(self.on_quit)
        self.menu.addAction(self.quit_action)

        # Set the menu on the tray icon
        self.tray.setContextMenu(self.menu)

        event_bus.on("PlatformService.battery_threshold", self.set_battery_charge_limit)
        event_bus.on("PlatformService.thermal_throttle_profile", self.set_thermal_throttle_policy)
        event_bus.on("OpenRgbService.aura_changed", self.set_aura_state)

    def set_battery_charge_limit(self, value: BatteryThreshold):
        """Set battery charge limit"""
        self.threshold_actions[value].setChecked(True)

    def set_thermal_throttle_policy(self, value: ThermalThrottleProfile):
        """Set performance profile"""
        self.logger.debug("Refreshing throttle policy in UI")
        self.performance_actions[value].setChecked(True)

    def set_aura_state(self, value):
        """Set aura state"""
        effect, brightness, color = value

        self.effect_actions[effect].setChecked(True)
        self.brightness_actions[brightness].setChecked(True)
        if color is None:
            self.color_menu.setDisabled(True)
        else:
            self.color_menu.setDisabled(False)
            self.color_action.setText(color)

    def show(self):
        """Show tray"""
        self.tray.setVisible(True)

    def on_threshold_selected(self, threshold: BatteryThreshold):
        """Battery limit event handler"""
        platform_service.set_battery_threshold(threshold)

    def on_effect_selected(self, effect: str):
        """Effect event handler"""
        open_rgb_service.apply_effect(effect)

    def on_brightness_selected(self, brightness: RgbBrightness):
        """Brightness event handler"""
        open_rgb_service.apply_brightness(brightness)

    def on_profile_selected(self, profile: ThermalThrottleProfile):
        """Profile event handler"""
        platform_service.set_thermal_throttle_policy(profile)

    def pick_color(self):
        """Open color picker"""
        color = QColorDialog.getColor(QColor(self.color_action.text()), None, translator.translate("color.select"))
        if color.isValid():
            open_rgb_service.apply_color(color.name().upper())

    @staticmethod
    def on_open():
        """Open main window"""
        main_window.show()

    @staticmethod
    def on_open_logs():
        """Open log file"""
        subprocess.run(["xdg-open", log_file], check=False)

    @staticmethod
    def on_quit():
        """Quit application"""
        event_bus.emit("stop", None)
        os.kill(os.getpid(), signal.SIGKILL)


tray_icon = TrayIcon()
