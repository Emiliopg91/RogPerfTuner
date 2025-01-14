import os
import signal
import subprocess

# pylint: disable=E0611, E0401
from PyQt5.QtWidgets import QSystemTrayIcon, QMenu, QAction, QActionGroup, QColorDialog
from PyQt5.QtGui import QIcon, QColor

from lib.gui.game_list import GameList
from lib.gui.main_window import main_window

from lib import __app_name__, __version__
from lib.models.boost import Boost
from lib.models.thermal_throttle_profile import ThermalThrottleProfile
from lib.models.rgb_brightness import RgbBrightness
from lib.models.battery_threshold import BatteryThreshold
from lib.services.games_service import games_service
from lib.services.openrgb_service import open_rgb_service
from lib.services.platform_service import platform_service
from lib.utils.constants import icons_path, log_file, dev_mode
from lib.utils.event_bus import event_bus
from lib.utils.logger import Logger
from lib.utils.singleton import singleton
from lib.utils.translator import translator


@singleton
class TrayIcon:  # pylint: disable=R0902
    """Tray icon class"""

    def __init__(self):  # pylint: disable=R0915
        self._logger = Logger()

        icon = QIcon.fromTheme(os.path.join(icons_path, "icon-45x45.png"))

        self._tray = QSystemTrayIcon()
        self._tray.setIcon(icon)
        self._tray.setToolTip(f"{__app_name__} v{__version__}")

        # Create the menu
        self._menu = QMenu()

        # Add "Battery" option (disabled)
        self._battery_action = QAction(translator.translate("battery"))
        self._battery_action.setEnabled(False)
        self._menu.addAction(self._battery_action)

        # Add "Umbral" submenu
        self._umbral_menu = QMenu("    " + translator.translate("charge.threshold"))
        self._threshold_group = QActionGroup(self._umbral_menu)
        self._threshold_group.setExclusive(True)
        self.threshold_actions: dict[BatteryThreshold, QAction] = {}
        for threshold in BatteryThreshold:
            action = QAction(f"{threshold.value}%", checkable=True)
            action.setActionGroup(self._threshold_group)
            action.setChecked(threshold == platform_service._battery_charge_limit)
            action.triggered.connect(lambda _, t=threshold: self.on_threshold_selected(t))
            self.threshold_actions[threshold] = action
            self._umbral_menu.addAction(action)
        self._menu.addMenu(self._umbral_menu)

        self._menu.addSeparator()

        # Add "AuraSync" option (disabled)
        self._aura_section = QAction("AuraSync")
        self._aura_section.setEnabled(False)
        self._menu.addAction(self._aura_section)

        # Add "Effect" submenu
        self._effect_menu = QMenu("    " + translator.translate("effect"))
        self._effect_group = QActionGroup(self._effect_menu)
        self._effect_group.setExclusive(True)
        self._effect_actions: dict[str, QAction] = {}
        for effect in open_rgb_service.get_available_effects():
            action = QAction(effect, checkable=True)
            action.setActionGroup(self._effect_group)
            action.setChecked(effect == open_rgb_service._effect)
            action.triggered.connect(lambda _, e=effect: self.on_effect_selected(e))
            self._effect_actions[effect] = action
            self._effect_menu.addAction(action)
        self._menu.addMenu(self._effect_menu)

        # Add "Brightness" submenu
        self._brightness_menu = QMenu("    " + translator.translate("brightness"))
        self._brightness_group = QActionGroup(self._brightness_menu)
        self._brightness_group.setExclusive(True)
        self._brightness_actions: dict[str, RgbBrightness] = {}
        for brightness in RgbBrightness:
            action = QAction(
                translator.translate(f"label.brightness.{brightness.name}"),
                checkable=True,
            )
            action.setActionGroup(self._brightness_group)
            action.setChecked(brightness == open_rgb_service._brightness)
            action.triggered.connect(lambda _, b=brightness: self.on_brightness_selected(b))
            self._brightness_actions[brightness] = action
            self._brightness_menu.addAction(action)
        self._menu.addMenu(self._brightness_menu)

        # Add "Color" submenu
        self._color_menu = QMenu("    " + translator.translate("color"))

        self._color_action = QAction(open_rgb_service.get_color())
        self._color_action.setEnabled(False)
        self._color_menu.addAction(self._color_action)

        self._colorpicker_action = QAction(f"{translator.translate("select.color")}...")
        self._colorpicker_action.triggered.connect(self.pick_color)
        self._color_menu.addAction(self._colorpicker_action)

        self._submenu_color = self._menu.addMenu(self._color_menu)
        self._submenu_color.setVisible(open_rgb_service.supports_color())

        self._menu.addSeparator()

        # Add "Performance" option
        self._performance_section = QAction(translator.translate("performance"))
        self._performance_section.setEnabled(False)
        self._menu.addAction(self._performance_section)

        self._profile_menu = QMenu("    " + translator.translate("profile"))
        self._boost_group = QActionGroup(self._profile_menu)
        self._boost_group.setExclusive(True)
        self._performance_actions: dict[ThermalThrottleProfile, QAction] = {}
        for profile in ThermalThrottleProfile:
            action = QAction(translator.translate(f"label.profile.{profile.name}"), checkable=True)
            action.setActionGroup(self._boost_group)
            action.setChecked(profile == platform_service._thermal_throttle_profile)
            action.triggered.connect(lambda _, p=profile: self.on_profile_selected(p))
            self._performance_actions[profile] = action
            self._profile_menu.addAction(action)
        self._menu.addMenu(self._profile_menu)

        # Add "Boost" option
        self._boost_menu = QMenu("    " + translator.translate("boost"))
        self._boost_group = QActionGroup(self._boost_menu)
        self._boost_group.setExclusive(True)
        self._boost_actions: dict[Boost, QAction] = {}
        for mode in Boost:
            action = QAction(translator.translate(f"label.boost.{mode.name}"), checkable=True)
            action.setActionGroup(self._boost_group)
            action.setChecked(mode == platform_service.get_boost_mode())
            action.triggered.connect(lambda _, m=mode: self.on_boost_selected(m))
            self._boost_actions[mode] = action
            self._boost_menu.addAction(action)
        self._menu.addMenu(self._boost_menu)

        if games_service.rccdc_enabled:
            # Add "Games" option
            self._games_menu = QMenu("    " + translator.translate("games"))
            self._select_profile_action = QAction(f"{translator.translate("label.game.configure")}...")
            self._select_profile_action.triggered.connect(self.on_open_game_list)
            self._games_menu.addAction(self._select_profile_action)
            self._menu.addMenu(self._games_menu)

        self._menu.addSeparator()

        if dev_mode:
            # Add "Open log" option
            self._open_logs_action = QAction(translator.translate("open.logs"))
            self._open_logs_action.triggered.connect(self.on_open_logs)
            self._menu.addAction(self._open_logs_action)

            self._menu.addSeparator()

        # Add "Open" option
        self._open_action = QAction(translator.translate("open.ui"))
        self._open_action.triggered.connect(self.on_open)
        self._menu.addAction(self._open_action)

        self._menu.addSeparator()

        # Add "Quit" option
        self._quit_action = QAction(translator.translate("close"))
        self._quit_action.triggered.connect(self.on_quit)
        self._menu.addAction(self._quit_action)

        # Set the menu on the tray icon
        self._tray.setContextMenu(self._menu)

        event_bus.on("PlatformService.battery_threshold", self.set_battery_charge_limit)
        event_bus.on("PlatformService.boost", self.set_boost_mode)
        event_bus.on("PlatformService.thermal_throttle_profile", self.set_thermal_throttle_policy)
        event_bus.on("OpenRgbService.aura_changed", self.set_aura_state)
        event_bus.on("GamesService.gameEvent", self.on_game_event)

        self._tray.activated.connect(self.on_tray_icon_activated)

    def on_game_event(self, running_games: int):
        """Handler for game events"""
        enable = running_games == 0
        self._profile_menu.setEnabled(enable)
        self._boost_menu.setEnabled(enable)
        self._games_menu.setEnabled(enable)

    def on_tray_icon_activated(self, reason):
        """Restore main window"""
        if reason == QSystemTrayIcon.Trigger:
            self.on_open()

    def set_battery_charge_limit(self, value: BatteryThreshold):
        """Set battery charge limit"""
        self.threshold_actions[value].setChecked(True)

    def set_boost_mode(self, value: Boost):
        """Set boost mode"""
        self._boost_actions[value].setChecked(True)

    def set_thermal_throttle_policy(self, value: ThermalThrottleProfile):
        """Set performance profile"""
        self._logger.debug("Refreshing throttle policy in UI")
        self._performance_actions[value].setChecked(True)

    def set_aura_state(self, value):
        """Set aura state"""
        effect, brightness, color = value

        self._effect_actions[effect].setChecked(True)
        self._brightness_actions[brightness].setChecked(True)
        if color is None:
            self._submenu_color.setVisible(False)
        else:
            self._submenu_color.setVisible(True)
            self._color_action.setText(color)

    def show(self):
        """Show tray"""
        self._tray.setVisible(True)

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

    def on_boost_selected(self, boost: Boost):
        """Boost event handler"""
        platform_service.set_boost_mode(boost)

    def pick_color(self):
        """Open color picker"""
        color = QColorDialog.getColor(QColor(self._color_action.text()), None, translator.translate("color.select"))
        if color.isValid():
            open_rgb_service.apply_color(color.name().upper())

    def on_open_game_list(self):
        """Open game list window"""
        GameList().show()

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
