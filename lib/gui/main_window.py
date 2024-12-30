from ..models.battery_threshold import BatteryThreshold
from ..models.rgb_brightness import RgbBrightness
from ..models.thermal_throttle_profile import ThermalThrottleProfile
from ..services.openrgb_service import open_rgb_service
from ..services.platform_service import platform_service
from ..utils.constants import icons_path
from ..utils.event_bus import event_bus
from ..utils.logger import Logger
from ..utils.translator import translator

from PyQt5.QtWidgets import (
    QApplication,
    QVBoxLayout,
    QFormLayout,
    QLabel,
    QComboBox,
    QGroupBox,
    QMainWindow,
    QWidget,
    QPushButton,
    QColorDialog,
)
from PyQt5.QtGui import QPixmap, QColor, QCursor, QIcon
from PyQt5.QtCore import Qt


class MainWindow(QMainWindow):
    def __init__(self):
        self.logger = Logger("MainWindow")

        self.profile_items = [profile for profile in ThermalThrottleProfile]
        self.profile_labels = [
            translator.translate(f"label.profile.{profile.name}")
            for profile in ThermalThrottleProfile
        ]
        self.effect_labels = open_rgb_service.get_available_effects()
        self.brightness_items = [brightness for brightness in RgbBrightness]
        self.brightness_labels = [
            translator.translate(f"label.brightness.{brightness.name}")
            for brightness in RgbBrightness
        ]
        self.battery_items = [limit for limit in BatteryThreshold]
        self.battery_labels = [f"{limit.value}%" for limit in BatteryThreshold]

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
        scaled_pixmap = pixmap.scaled(
            350, 350, Qt.KeepAspectRatio, Qt.SmoothTransformation
        )  # Escalar la imagen
        image_label.setPixmap(scaled_pixmap)
        image_label.setAlignment(Qt.AlignCenter)

        main_layout.addWidget(
            image_label, alignment=Qt.AlignCenter
        )  # Centrar la imagen

        # Grupo 1: Rendimiento
        performance_group = QGroupBox(translator.translate("performance"))
        performance_layout = QFormLayout()
        performance_layout.setContentsMargins(20, 20, 20, 20)

        # Label y Dropdown para "Perfil"
        self.profile_dropdown = QComboBox()
        self.profile_dropdown.addItems(self.profile_labels)
        self.profile_dropdown.currentIndexChanged.connect(self.on_profile_changed)
        self.set_thermal_throttle_policy(
            platform_service.get_thermal_throttle_profile()
        )
        performance_layout.addRow(
            QLabel(f"{translator.translate('profile')}:"), self.profile_dropdown
        )

        performance_group.setLayout(performance_layout)
        main_layout.addWidget(performance_group)

        # Grupo 2: Aura
        aura_group = QGroupBox("Aura")
        aura_layout = QFormLayout()
        aura_layout.setContentsMargins(20, 20, 20, 20)

        # Dropdown para "Efecto"
        self.effect_dropdown = QComboBox()
        self.effect_dropdown.addItems(self.effect_labels)
        self.effect_dropdown.setCurrentIndex(
            self.effect_labels.index(open_rgb_service.effect)
        )
        self.effect_dropdown.currentIndexChanged.connect(self.on_effect_change)
        aura_layout.addRow(
            QLabel(f"{translator.translate("effect")}:"), self.effect_dropdown
        )

        # Dropdown para "Brillo"
        self.brightness_dropdown = QComboBox()
        self.brightness_dropdown.addItems(self.brightness_labels)
        self.brightness_dropdown.setCurrentIndex(
            self.brightness_items.index(open_rgb_service.brightness)
        )
        self.brightness_dropdown.currentIndexChanged.connect(self.on_brightness_change)
        aura_layout.addRow(
            QLabel(f"{translator.translate("brightness")}:"), self.brightness_dropdown
        )

        # Botón de color
        self.color_button = QPushButton()
        self.color_button.setFixedSize(50, 50)
        self.color_button.setStyleSheet(
            f"background-color: {self.current_color if self.current_color is not None else "#00000000"};"
        )
        self.color_button.clicked.connect(self.pick_color)
        self.color_button.setDisabled(
            not open_rgb_service.supports_color(open_rgb_service.effect)
        )
        aura_layout.addRow(
            QLabel(f"{translator.translate('color')}:"), self.color_button
        )

        aura_group.setLayout(aura_layout)
        main_layout.addWidget(aura_group)

        # Grupo 3: Configuración
        settings_group = QGroupBox(translator.translate("battery"))
        settings_layout = QFormLayout()
        settings_layout.setContentsMargins(20, 20, 20, 20)

        # Dropdown para "Umbral"
        self.threshold_dropdown = QComboBox()
        self.threshold_dropdown.addItems(self.battery_labels)
        self.threshold_dropdown.currentIndexChanged.connect(
            self.on_battery_limit_changed
        )
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
        self.center_window_on_current_screen()

        event_bus.on("PlatformService.battery_threshold", self.set_battery_charge_limit)
        event_bus.on(
            "PlatformService.thermal_throttle_profile", self.set_thermal_throttle_policy
        )
        event_bus.on("OpenRgbService.aura_changed", self.set_aura_state)

    def closeEvent(self, event):
        """Override the close event to hide the window instead of closing it."""
        event.ignore()
        self.hide()

    def pick_color(self):
        color = QColorDialog.getColor(
            QColor(self.current_color), self, translator.translate("color.select")
        )
        if color.isValid():
            self.current_color = color.name().upper()
            self.color_button.setStyleSheet(f"background-color: {self.current_color};")
            self.on_color_change()

    def center_window_on_current_screen(self):
        cursor_position = QCursor.pos()
        screen = QApplication.screenAt(cursor_position)
        if screen is not None:
            screen_geometry = screen.availableGeometry()
            x = int((screen_geometry.width() - self.width()) / 2) + screen_geometry.x()
            y = (
                int((screen_geometry.height() - self.height()) / 2)
                + screen_geometry.y()
            )
            self.move(x, y)

    def set_thermal_throttle_policy(self, value: ThermalThrottleProfile):
        self.profile_dropdown.setCurrentIndex(self.profile_items.index(value))

    def set_battery_charge_limit(self, value: BatteryThreshold):
        self.threshold_dropdown.setCurrentIndex(self.battery_items.index(value))

    def set_aura_state(self, value):
        effect, brightness, color = value

        self.effect_dropdown.setCurrentIndex(self.effect_labels.index(effect))
        self.brightness_dropdown.setCurrentIndex(
            self.brightness_items.index(brightness)
        )
        self.current_color = color if color is not None else "#00000000"
        self.color_button.setStyleSheet(f"background-color: {self.current_color};")
        self.color_button.setDisabled(color is None)

    def on_profile_changed(self):
        index = self.profile_dropdown.currentIndex()
        platform_service.set_thermal_throttle_policy(self.profile_items[index])

    def on_battery_limit_changed(self):
        index = self.threshold_dropdown.currentIndex()
        platform_service.set_battery_threshold(self.battery_items[index])

    def on_effect_change(self):
        effect = self.effect_labels[self.effect_dropdown.currentIndex()]
        open_rgb_service.apply_effect(effect)

    def on_brightness_change(self):
        brightness = self.brightness_items[self.brightness_dropdown.currentIndex()]
        open_rgb_service.apply_brightness(brightness)

    def on_color_change(self):
        open_rgb_service.apply_color(self.current_color)


main_window = MainWindow()
