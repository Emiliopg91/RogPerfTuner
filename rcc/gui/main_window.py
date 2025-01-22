# pylint: disable=E0611, E0401
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor, QIcon, QPixmap
from PyQt5.QtWidgets import (
    QColorDialog,
    QComboBox,
    QFormLayout,
    QGroupBox,
    QLabel,
    QMainWindow,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

from rcc.gui.game_list import GameList
from rcc.models.battery_threshold import BatteryThreshold
from rcc.models.boost import Boost
from rcc.models.rgb_brightness import RgbBrightness
from rcc.models.platform_profile import PlatformProfile
from rcc.services.openrgb_service import open_rgb_service
from rcc.services.platform_service import platform_service
from rcc.utils.constants import icons_path
from rcc.utils.event_bus import event_bus
from rcc.utils.gui_utils import GuiUtils
from rcc.utils.logger import Logger
from rcc.utils.translator import translator


class MainWindow(QMainWindow):
    """Aplication main window"""

    def __init__(self):
        self._logger = Logger()

        self._effect_labels = open_rgb_service.get_available_effects()

        super().__init__()

        self.setWindowTitle("RogControlCenter")
        self.setGeometry(0, 0, 580, 800)
        self.setFixedSize(580, 800)
        self.setWindowIcon(QIcon(f"{icons_path}/icon-45x45.png"))

        self._current_color = open_rgb_service.get_color(open_rgb_service._effect)

        # Widget central
        central_widget = QWidget(self)
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(20, 20, 20, 20)
        main_layout.setAlignment(Qt.AlignTop)  # Alinear los elementos al inicio

        # Primera fila: Imagen centrada
        image_label = QLabel()
        pixmap = QPixmap(f"{icons_path}/rog-logo.svg")  # Ruta a tu archivo de imagen
        scaled_pixmap = pixmap.scaled(350, 350, Qt.KeepAspectRatio, Qt.SmoothTransformation)  # Escalar la imagen
        image_label.setPixmap(scaled_pixmap)
        image_label.setAlignment(Qt.AlignCenter)

        main_layout.addWidget(image_label, alignment=Qt.AlignCenter)  # Centrar la imagen

        # Grupo 1: Rendimiento
        performance_group = QGroupBox(translator.translate("performance"))
        performance_layout = QFormLayout()
        performance_layout.setContentsMargins(20, 20, 20, 20)

        # Label y Dropdown para "Perfil"
        self._profile_dropdown = QComboBox()
        for item in PlatformProfile:
            self._profile_dropdown.addItem(translator.translate(f"label.profile.{item.name}"), item)
        self._profile_dropdown.currentIndexChanged.connect(self.on_profile_changed)
        self.set_thermal_throttle_policy(platform_service.get_thermal_throttle_profile())
        performance_layout.addRow(QLabel(f"{translator.translate('profile')}:"), self._profile_dropdown)

        # Label y Dropdown para "Boost"
        self._boost_dropdown = QComboBox()
        for item in Boost:
            self._boost_dropdown.addItem(translator.translate(f"label.boost.{item.name}"), item)
        self._boost_dropdown.currentIndexChanged.connect(self.on_boost_changed)
        self.set_boost_mode(platform_service.boost_mode)
        performance_layout.addRow(QLabel(f"{translator.translate('boost')}:"), self._boost_dropdown)

        performance_group.setLayout(performance_layout)
        main_layout.addWidget(performance_group)

        self._game_profile_button = QPushButton(translator.translate("label.game.configure"))
        self._game_profile_button.clicked.connect(self.open_game_list)
        performance_layout.addRow(QLabel(f"{translator.translate('profile.per.game')}:"), self._game_profile_button)

        # Grupo 2: Aura
        aura_group = QGroupBox("Aura")
        aura_layout = QFormLayout()
        aura_layout.setContentsMargins(20, 20, 20, 20)

        # Dropdown para "Efecto"
        self._effect_dropdown = QComboBox()
        self._effect_dropdown.addItems(self._effect_labels)
        self._effect_dropdown.setCurrentIndex(self._effect_labels.index(open_rgb_service._effect))
        self._effect_dropdown.currentIndexChanged.connect(self.on_effect_change)
        aura_layout.addRow(QLabel(f"{translator.translate("effect")}:"), self._effect_dropdown)

        # Dropdown para "Brillo"
        self._brightness_dropdown = QComboBox()
        for brightness in RgbBrightness:
            self._brightness_dropdown.addItem(translator.translate(f"label.brightness.{brightness.name}"), brightness)
        self._brightness_dropdown.setCurrentIndex(self._brightness_dropdown.findData(open_rgb_service._brightness))
        self._brightness_dropdown.currentIndexChanged.connect(self.on_brightness_change)
        aura_layout.addRow(QLabel(f"{translator.translate("brightness")}:"), self._brightness_dropdown)

        # Botón de color
        self._color_button = QPushButton()
        self._color_button.setFixedSize(50, 30)
        self._color_button.setStyleSheet(
            f"background-color: {self._current_color if self._current_color is not None else "#00000000"};"
        )
        self._color_button.clicked.connect(self.pick_color)
        supports_color = open_rgb_service.supports_color(open_rgb_service._effect)
        self._color_button.setDisabled(not supports_color)
        self._color_button.setDisabled(not supports_color)

        self._color_label = QLabel(f"{translator.translate('color')}:")
        self._color_label.setDisabled(not supports_color)
        aura_layout.addRow(self._color_label, self._color_button)

        aura_group.setLayout(aura_layout)
        main_layout.addWidget(aura_group)

        # Grupo 3: Configuración
        settings_group = QGroupBox(translator.translate("battery"))
        settings_layout = QFormLayout()
        settings_layout.setContentsMargins(20, 20, 20, 20)

        # Dropdown para "Umbral"
        self._threshold_dropdown = QComboBox()
        for item in BatteryThreshold:
            self._threshold_dropdown.addItem(f"{item.value}%", item)
        self._threshold_dropdown.currentIndexChanged.connect(self.on_battery_limit_changed)
        self.set_battery_charge_limit(platform_service.battery_charge_limit)
        settings_layout.addRow(
            QLabel(
                f"{translator.translate("charge.threshold")}:",
            ),
            self._threshold_dropdown,
        )

        settings_group.setLayout(settings_layout)
        main_layout.addWidget(settings_group)

        # Configurar el widget central
        self.setCentralWidget(central_widget)
        GuiUtils.center_window_on_current_screen(self)

        event_bus.on("PlatformService.battery_threshold", self.set_battery_charge_limit)  # pylint: disable=R0801
        event_bus.on("PlatformService.boost", self.set_boost_mode)
        event_bus.on("PlatformService.thermal_throttle_profile", self.set_thermal_throttle_policy)
        event_bus.on("OpenRgbService.aura_changed", self.set_aura_state)
        event_bus.on("GamesService.gameEvent", self.on_game_event)

    def on_game_event(self, running_games: int):
        """Handler for game events"""
        enable = running_games == 0
        self._profile_dropdown.setEnabled(enable)
        self._boost_dropdown.setEnabled(enable)
        self._game_profile_button.setEnabled(enable)

    def closeEvent(self, event):  # pylint: disable=C0103
        """Override the close event to hide the window instead of closing it."""
        event.ignore()
        self.hide()

    def pick_color(self):
        """Open color picker"""
        color = QColorDialog.getColor(QColor(self._current_color), self, translator.translate("color.select"))
        if color.isValid():
            self._current_color = color.name().upper()
            self._color_button.setStyleSheet(f"background-color: {self._current_color};")
            self.on_color_change()

    def set_thermal_throttle_policy(self, value: PlatformProfile):
        """Set profile policy"""
        self._profile_dropdown.setCurrentIndex(self._profile_dropdown.findData(value))

    def set_boost_mode(self, value: Boost):
        """Set boost mode"""
        self._boost_dropdown.setCurrentIndex(self._boost_dropdown.findData(value))

    def set_battery_charge_limit(self, value: BatteryThreshold):
        """Set battery limit"""
        self._threshold_dropdown.setCurrentIndex(self._threshold_dropdown.findData(value))

    def set_aura_state(self, value):
        """Set aura state"""
        effect, brightness, color = value

        self._effect_dropdown.setCurrentIndex(self._effect_labels.index(effect))
        self._brightness_dropdown.setCurrentIndex(self._brightness_dropdown.findData(brightness))
        self._current_color = color if color is not None else "#00000000"
        self._color_button.setStyleSheet(f"background-color: {self._current_color};")
        self._color_button.setDisabled(color is None)
        self._color_button.setDisabled(color is None)
        self._color_label.setDisabled(color is None)

    def on_profile_changed(self, index):
        """Handler for profile change"""
        profile = self._profile_dropdown.itemData(index)
        if platform_service.thermal_throttle_profile != profile:
            platform_service.set_thermal_throttle_policy(profile)

    def on_boost_changed(self, index):
        """Handler for boost change"""
        mode = self._boost_dropdown.itemData(index)
        if platform_service.boost_mode != mode:
            platform_service.set_boost_mode(mode)

    def on_battery_limit_changed(self, index):
        """Handler for battery limit change"""
        threshold = self._threshold_dropdown.itemData(index)
        if platform_service.battery_charge_limit != threshold:
            platform_service.set_battery_threshold(threshold)

    def on_effect_change(self):
        """Handler for effect change"""
        effect = self._effect_labels[self._effect_dropdown.currentIndex()]
        if open_rgb_service.effect != effect:
            open_rgb_service.apply_effect(effect)

    def on_brightness_change(self, index):
        """Handler for brightness change"""
        level = self._brightness_dropdown.itemData(index)
        if open_rgb_service.brightness != level:
            open_rgb_service.apply_brightness(level)

    def on_color_change(self):
        """Handler for color change"""
        if open_rgb_service.color != self._current_color:
            open_rgb_service.apply_color(self._current_color)

    def open_game_list(self):
        """Open game list window"""
        GameList(self).show()


main_window = MainWindow()
