# pylint: disable=duplicate-code

from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor, QIcon, QPixmap
from PyQt5.QtWidgets import (
    QColorDialog,
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
from rcc.models.performance_profile import PerformanceProfile
from rcc.models.rgb_brightness import RgbBrightness
from rcc.services.hardware_service import HARDWARE_SERVICE
from rcc.services.steam_service import STEAM_SERVICE
from rcc.services.rgb_service import RGB_SERVICE
from rcc.services.profile_service import PROFILE_SERVICE
from rcc.utils.constants import ICONS_PATH
from rcc.utils.beans import TRANSLATOR
from rcc.utils.events import (
    OPENRGB_SERVICE_AURA_CHANGED,
    HARDWARE_SERVICE_BATTERY_THRESHOLD_CHANGED,
    PLATFORM_SERVICE_PROFILE_CHANGED,
    STEAM_SERVICE_CONNECTED,
    STEAM_SERVICE_DISCONNECTED,
    STEAM_SERVICE_GAME_EVENT,
)
from rcc.utils.gui_utils import GuiUtils, NoScrollComboBox
from rcc.utils.beans import EVENT_BUS
from framework.logger import Logger


class MainWindow(QMainWindow):
    """Aplication main window"""

    def __init__(self):
        self._logger = Logger()

        self._effect_labels = RGB_SERVICE.get_available_effects()

        super().__init__()

        self.setWindowTitle("RogControlCenter")
        self.setGeometry(0, 0, 350, 600)
        self.setFixedSize(350, 600)
        self.setWindowIcon(QIcon(f"{ICONS_PATH}/icon-45x45.png"))

        self._current_color = RGB_SERVICE.get_color(RGB_SERVICE._effect)

        # Widget central
        central_widget = QWidget(self)
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(20, 20, 20, 20)
        main_layout.setAlignment(Qt.AlignTop)  # Alinear los elementos al inicio

        # Primera fila: Imagen centrada
        image_label = QLabel()
        pixmap = QPixmap(f"{ICONS_PATH}/rog-logo.svg")  # Ruta a tu archivo de imagen
        scaled_pixmap = pixmap.scaled(200, 200, Qt.KeepAspectRatio, Qt.SmoothTransformation)  # Escalar la imagen
        image_label.setPixmap(scaled_pixmap)
        image_label.setAlignment(Qt.AlignCenter)

        main_layout.addWidget(image_label, alignment=Qt.AlignCenter)  # Centrar la imagen

        # Grupo 1: Rendimiento
        performance_group = QGroupBox(TRANSLATOR.translate("performance"))
        performance_layout = QFormLayout()
        performance_layout.setContentsMargins(20, 20, 20, 20)

        # Label y Dropdown para "Perfil"
        self._profile_dropdown = NoScrollComboBox()
        for item in reversed(PerformanceProfile):
            self._profile_dropdown.addItem(TRANSLATOR.translate(f"label.profile.{item.name}"), item)
        self._profile_dropdown.currentIndexChanged.connect(self.on_profile_changed)
        self.set_performance_profile(PROFILE_SERVICE.performance_profile)
        performance_layout.addRow(QLabel(f"{TRANSLATOR.translate('profile')}:"), self._profile_dropdown)

        performance_group.setLayout(performance_layout)
        main_layout.addWidget(performance_group)

        self._game_profile_button = QPushButton(TRANSLATOR.translate("label.game.configure"))
        self._game_profile_button.clicked.connect(self.open_game_list)
        self._game_profile_button.setEnabled(STEAM_SERVICE.steam_connected)
        performance_layout.addRow(QLabel(f"{TRANSLATOR.translate('games')}:"), self._game_profile_button)

        # Grupo 2: Aura
        aura_group = QGroupBox("Aura")
        aura_layout = QFormLayout()
        aura_layout.setContentsMargins(20, 20, 20, 20)

        # Dropdown para "Efecto"
        self._effect_dropdown = NoScrollComboBox()
        self._effect_dropdown.addItems(self._effect_labels)
        if RGB_SERVICE._effect in self._effect_labels:
            self._effect_dropdown.setCurrentIndex(self._effect_labels.index(RGB_SERVICE._effect))
        self._effect_dropdown.currentIndexChanged.connect(self.on_effect_change)
        aura_layout.addRow(QLabel(f"{TRANSLATOR.translate("effect")}:"), self._effect_dropdown)

        # Dropdown para "Brillo"
        self._brightness_dropdown = NoScrollComboBox()
        for brightness in RgbBrightness:
            self._brightness_dropdown.addItem(TRANSLATOR.translate(f"label.brightness.{brightness.name}"), brightness)
        self._brightness_dropdown.setCurrentIndex(self._brightness_dropdown.findData(RGB_SERVICE._brightness))
        self._brightness_dropdown.currentIndexChanged.connect(self.on_brightness_change)
        aura_layout.addRow(QLabel(f"{TRANSLATOR.translate("brightness")}:"), self._brightness_dropdown)

        # Botón de color
        self._color_button = QPushButton()
        self._color_button.setFixedSize(25, 25)
        self._color_button.setStyleSheet(
            f"background-color: {self._current_color if self._current_color is not None else "#00000000"};"
        )
        self._color_button.clicked.connect(self.pick_color)
        supports_color = RGB_SERVICE.supports_color(RGB_SERVICE._effect)
        self._color_button.setDisabled(not supports_color)
        self._color_button.setDisabled(not supports_color)

        self._color_label = QLabel(f"{TRANSLATOR.translate('color')}:")
        self._color_label.setDisabled(not supports_color)
        aura_layout.addRow(self._color_label, self._color_button)

        aura_group.setLayout(aura_layout)
        main_layout.addWidget(aura_group)

        # Grupo 3: Configuración
        settings_group = QGroupBox(TRANSLATOR.translate("battery"))
        settings_layout = QFormLayout()
        settings_layout.setContentsMargins(20, 20, 20, 20)

        # Dropdown para "Umbral"
        self._threshold_dropdown = NoScrollComboBox()
        for item in BatteryThreshold:
            self._threshold_dropdown.addItem(f"{item.value}%", item)
        self._threshold_dropdown.currentIndexChanged.connect(self.on_battery_limit_changed)
        self.set_battery_charge_limit(HARDWARE_SERVICE.battery_charge_limit)
        settings_layout.addRow(
            QLabel(
                f"{TRANSLATOR.translate("charge.threshold")}:",
            ),
            self._threshold_dropdown,
        )

        settings_group.setLayout(settings_layout)
        main_layout.addWidget(settings_group)

        # Configurar el widget central
        self.setCentralWidget(central_widget)
        GuiUtils.center_window_on_current_screen(self)

        EVENT_BUS.on(
            HARDWARE_SERVICE_BATTERY_THRESHOLD_CHANGED, self.set_battery_charge_limit
        )  # pylint: disable=duplicate-code
        EVENT_BUS.on(PLATFORM_SERVICE_PROFILE_CHANGED, self.set_performance_profile)
        EVENT_BUS.on(OPENRGB_SERVICE_AURA_CHANGED, self.set_aura_state)
        EVENT_BUS.on(STEAM_SERVICE_GAME_EVENT, self.on_game_event)
        EVENT_BUS.on(STEAM_SERVICE_CONNECTED, lambda: self.on_steam_connected_event(True))
        EVENT_BUS.on(STEAM_SERVICE_DISCONNECTED, lambda: self.on_steam_connected_event(False))

    def on_steam_connected_event(self, connected: bool):
        """Handler for steam connection events"""
        self._game_profile_button.setEnabled(connected)

    def on_game_event(self, running_games: int):
        """Handler for game events"""
        enable = running_games == 0
        self._profile_dropdown.setEnabled(enable)
        self._game_profile_button.setEnabled(enable)

    def closeEvent(self, event):  # pylint: disable=invalid-name
        """Override the close event to hide the window instead of closing it."""
        event.ignore()
        self.hide()

    def pick_color(self):
        """Open color picker"""
        color = QColorDialog.getColor(QColor(self._current_color), self, TRANSLATOR.translate("color.select"))
        if color.isValid():
            self._current_color = color.name().upper()
            self._color_button.setStyleSheet(f"background-color: {self._current_color};")
            self.on_color_change()

    def set_performance_profile(self, value: PerformanceProfile):
        """Set profile policy"""
        self._profile_dropdown.setCurrentIndex(self._profile_dropdown.findData(value))

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
        if len(STEAM_SERVICE.running_games.keys()) == 0:
            if PROFILE_SERVICE.performance_profile != profile:
                PROFILE_SERVICE.set_performance_profile(profile)

    def on_battery_limit_changed(self, index):
        """Handler for battery limit change"""
        threshold = self._threshold_dropdown.itemData(index)
        if HARDWARE_SERVICE.battery_charge_limit != threshold:
            HARDWARE_SERVICE.set_battery_threshold(threshold)

    def on_effect_change(self):
        """Handler for effect change"""
        effect = self._effect_labels[self._effect_dropdown.currentIndex()]
        if RGB_SERVICE.effect != effect:
            RGB_SERVICE.apply_effect(effect)

    def on_brightness_change(self, index):
        """Handler for brightness change"""
        level = self._brightness_dropdown.itemData(index)
        if RGB_SERVICE.brightness != level:
            RGB_SERVICE.apply_brightness(level)

    def on_color_change(self):
        """Handler for color change"""
        if RGB_SERVICE.color != self._current_color:
            RGB_SERVICE.apply_color(self._current_color)

    def open_game_list(self):
        """Open game list window"""
        GameList(self).show()


MAIN_WINDOW = MainWindow()
