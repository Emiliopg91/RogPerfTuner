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

from lib.models.battery_threshold import BatteryThreshold
from lib.models.boost import Boost
from lib.models.rgb_brightness import RgbBrightness
from lib.models.thermal_throttle_profile import ThermalThrottleProfile
from lib.services.openrgb_service import open_rgb_service
from lib.services.platform_service import platform_service
from lib.utils.constants import icons_path
from lib.utils.event_bus import event_bus
from lib.utils.gui_utils import GuiUtils
from lib.utils.logger import Logger
from lib.utils.translator import translator


class MainWindow(QMainWindow):
    """Aplication main window"""

    def __init__(self):
        self.logger = Logger()

        self.effect_labels = open_rgb_service.get_available_effects()

        super().__init__()

        self.setWindowTitle("RogControlCenter")
        self.setGeometry(0, 0, 580, 800)
        self.setFixedSize(580, 800)
        self.setWindowIcon(QIcon(f"{icons_path}/icon-45x45.png"))

        self.current_color = open_rgb_service.get_color(open_rgb_service.effect)

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
        self.profile_dropdown = QComboBox()
        for item in ThermalThrottleProfile:
            self.profile_dropdown.addItem(translator.translate(f"label.profile.{item.name}"), item)
        self.profile_dropdown.currentIndexChanged.connect(self.on_profile_changed)
        self.set_thermal_throttle_policy(platform_service.get_thermal_throttle_profile())
        performance_layout.addRow(QLabel(f"{translator.translate('profile')}:"), self.profile_dropdown)

        # Label y Dropdown para "Boost"
        self.boost_dropdown = QComboBox()
        for item in Boost:
            self.boost_dropdown.addItem(translator.translate(f"label.boost.{item.name}"), item)
        self.boost_dropdown.currentIndexChanged.connect(self.on_boost_changed)
        self.set_boost_mode(platform_service.get_boost_mode())
        performance_layout.addRow(QLabel(f"{translator.translate('boost')}:"), self.boost_dropdown)

        performance_group.setLayout(performance_layout)
        main_layout.addWidget(performance_group)

        # Grupo 2: Aura
        aura_group = QGroupBox("Aura")
        aura_layout = QFormLayout()
        aura_layout.setContentsMargins(20, 20, 20, 20)

        # Dropdown para "Efecto"
        self.effect_dropdown = QComboBox()
        self.effect_dropdown.addItems(self.effect_labels)
        self.effect_dropdown.setCurrentIndex(self.effect_labels.index(open_rgb_service.effect))
        self.effect_dropdown.currentIndexChanged.connect(self.on_effect_change)
        aura_layout.addRow(QLabel(f"{translator.translate("effect")}:"), self.effect_dropdown)

        # Dropdown para "Brillo"
        self.brightness_dropdown = QComboBox()
        for brightness in RgbBrightness:
            self.brightness_dropdown.addItem(translator.translate(f"label.brightness.{brightness.name}"), brightness)
        self.brightness_dropdown.setCurrentIndex(self.brightness_dropdown.findData(open_rgb_service.brightness))
        self.brightness_dropdown.currentIndexChanged.connect(self.on_brightness_change)
        aura_layout.addRow(QLabel(f"{translator.translate("brightness")}:"), self.brightness_dropdown)

        # Botón de color
        self.color_button = QPushButton()
        self.color_button.setFixedSize(50, 50)
        self.color_button.setStyleSheet(
            f"background-color: {self.current_color if self.current_color is not None else "#00000000"};"
        )
        self.color_button.clicked.connect(self.pick_color)
        supports_color = open_rgb_service.supports_color(open_rgb_service.effect)
        self.color_button.setDisabled(not supports_color)
        self.color_button.setHidden(not supports_color)

        self.color_label = QLabel(f"{translator.translate('color')}:")
        self.color_label.setHidden(not supports_color)
        aura_layout.addRow(self.color_label, self.color_button)

        aura_group.setLayout(aura_layout)
        main_layout.addWidget(aura_group)

        # Grupo 3: Configuración
        settings_group = QGroupBox(translator.translate("battery"))
        settings_layout = QFormLayout()
        settings_layout.setContentsMargins(20, 20, 20, 20)

        # Dropdown para "Umbral"
        self.threshold_dropdown = QComboBox()
        for item in BatteryThreshold:
            self.threshold_dropdown.addItem(f"{item.value}%", item)
        self.threshold_dropdown.currentIndexChanged.connect(self.on_battery_limit_changed)
        self.set_battery_charge_limit(platform_service.get_battery_charge_limit())
        settings_layout.addRow(
            QLabel(
                f"{translator.translate("charge.threshold")}:",
            ),
            self.threshold_dropdown,
        )

        settings_group.setLayout(settings_layout)
        main_layout.addWidget(settings_group)

        # Configurar el widget central
        self.setCentralWidget(central_widget)
        GuiUtils.center_window_on_current_screen(self)

        event_bus.on("PlatformService.battery_threshold", self.set_battery_charge_limit)
        event_bus.on("PlatformService.boost", self.set_boost_mode)
        event_bus.on("PlatformService.thermal_throttle_profile", self.set_thermal_throttle_policy)
        event_bus.on("OpenRgbService.aura_changed", self.set_aura_state)

    def closeEvent(self, event):  # pylint: disable=C0103
        """Override the close event to hide the window instead of closing it."""
        event.ignore()
        self.hide()

    def pick_color(self):
        """Open color picker"""
        color = QColorDialog.getColor(QColor(self.current_color), self, translator.translate("color.select"))
        if color.isValid():
            self.current_color = color.name().upper()
            self.color_button.setStyleSheet(f"background-color: {self.current_color};")
            self.on_color_change()

    def set_thermal_throttle_policy(self, value: ThermalThrottleProfile):
        """Set profile policy"""
        self.profile_dropdown.setCurrentIndex(self.profile_dropdown.findData(value))

    def set_boost_mode(self, value: Boost):
        """Set boost mode"""
        self.boost_dropdown.setCurrentIndex(self.boost_dropdown.findData(value))

    def set_battery_charge_limit(self, value: BatteryThreshold):
        """Set battery limit"""
        self.threshold_dropdown.setCurrentIndex(self.threshold_dropdown.findData(value))

    def set_aura_state(self, value):
        """Set aura state"""
        effect, brightness, color = value

        self.effect_dropdown.setCurrentIndex(self.effect_labels.index(effect))
        self.brightness_dropdown.setCurrentIndex(self.brightness_dropdown.findData(brightness))
        self.current_color = color if color is not None else "#00000000"
        self.color_button.setStyleSheet(f"background-color: {self.current_color};")
        self.color_button.setDisabled(color is None)
        self.color_button.setHidden(color is None)
        self.color_label.setHidden(color is None)

    def on_profile_changed(self, index):
        """Handler for profile change"""
        platform_service.set_thermal_throttle_policy(self.profile_dropdown.itemData(index))

    def on_boost_changed(self, index):
        """Handler for boost change"""
        platform_service.set_boost_mode(self.boost_dropdown.itemData(index))

    def on_battery_limit_changed(self, index):
        """Handler for battery limit change"""
        platform_service.set_battery_threshold(self.threshold_dropdown.itemData(index))

    def on_effect_change(self):
        """Handler for effect change"""
        effect = self.effect_labels[self.effect_dropdown.currentIndex()]
        open_rgb_service.apply_effect(effect)

    def on_brightness_change(self, index):
        """Handler for brightness change"""
        open_rgb_service.apply_brightness(self.brightness_dropdown.itemData(index))

    def on_color_change(self):
        """Handler for color change"""
        open_rgb_service.apply_color(self.current_color)


main_window = MainWindow()
