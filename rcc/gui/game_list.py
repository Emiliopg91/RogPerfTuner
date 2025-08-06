from PyQt5.QtWidgets import (
    QDialog,
    QVBoxLayout,
    QTableWidget,
    QTableWidgetItem,
    QScrollArea,
    QAbstractScrollArea,
    QHeaderView,
    QWidget,
    QAbstractItemView,
    QLineEdit,
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QIcon, QPixmap, QColor

from rcc.models.mangohud_level import MangoHudLevel
from rcc.models.wine_sync_option import WineSyncOption
from rcc.services.hardware_service import HARDWARE_SERVICE
from rcc.services.steam_service import STEAM_SERVICE
from rcc.utils.constants import ICONS_PATH
from rcc.utils.events import STEAM_SERVICE_CONNECTED
from rcc.utils.gui_utils import NoScrollComboBox
from rcc.utils.beans import EVENT_BUS, TRANSLATOR


class GameList(QDialog):
    """Window for game list"""

    INSTANCE: QDialog = None

    def __init__(
        self, parent: QWidget, manage_parent=False, app_id: int = None
    ):  # pylint: disable=too-many-locals, too-many-branches, too-many-statements
        super().__init__(parent)
        if GameList.INSTANCE is None:  # pylint: disable=too-many-nested-blocks
            GameList.INSTANCE = self
            self.__parent = parent
            self.__manage_parent = manage_parent
            self.setWindowTitle(TRANSLATOR.translate("game.performance.configuration"))
            self.setFixedSize(1000, 600)
            self.setWindowIcon(QIcon(f"{ICONS_PATH}/icon-45x45.png"))
            self.setAttribute(Qt.WA_DeleteOnClose)
            self.setWindowFlags(self.windowFlags() & ~Qt.WindowContextHelpButtonHint)

            # Configurar layout principal
            layout = QVBoxLayout(self)

            # Crear el área de scroll
            scroll_area = QScrollArea(self)
            scroll_area.setWidgetResizable(True)
            scroll_area.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
            scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarAsNeeded)

            # Crear un widget contenedor
            container = QWidget()
            scroll_area.setWidget(container)

            # Layout del contenedor
            container_layout = QVBoxLayout(container)

            # Crear la tabla
            game_cfg = STEAM_SERVICE.get_games()
            appids = list(game_cfg.keys())
            if app_id is not None:
                appids.append(app_id)

            columns = [
                TRANSLATOR.translate("game.title"),
                TRANSLATOR.translate("used.gpu"),
                TRANSLATOR.translate("used.steamdeck"),
            ]

            if STEAM_SERVICE.metrics_enabled:
                columns.append(TRANSLATOR.translate("metrics"))

            columns.append(TRANSLATOR.translate("winesync"))
            columns.append(TRANSLATOR.translate("environment"))
            columns.append(TRANSLATOR.translate("params"))

            table = QTableWidget(len(appids), len(columns))
            table.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)
            table.setHorizontalHeaderLabels(columns)
            table.verticalHeader().setVisible(False)

            # Configurar el ancho de las columnas
            header = table.horizontalHeader()
            # Primera columna se expande
            header.setSectionResizeMode(0, QHeaderView.Stretch)
            # Resto de columnas ajusta su tamaño al contenido
            for i in range(1, len(columns)):
                header.setSectionResizeMode(i, QHeaderView.ResizeToContents)
            header.setHighlightSections(False)  # Evitar que el encabezado se resalte

            # Desactivar edición y selección en la tabla
            table.setEditTriggers(QAbstractItemView.NoEditTriggers)
            table.setSelectionMode(QAbstractItemView.NoSelection)
            table.setFocusPolicy(Qt.NoFocus)

            row = 0
            for appid in sorted(appids, key=lambda e: game_cfg[e].name):
                is_running = appid in STEAM_SERVICE.running_games
                try:
                    col = 0
                    # Title

                    item = QTableWidgetItem(
                        game_cfg[appid].name
                        if not is_running
                        else game_cfg[appid].name + " (" + TRANSLATOR.translate("running") + "...)"
                    )
                    if STEAM_SERVICE.get_icon_path(appid):
                        pixmap = QPixmap(STEAM_SERVICE.get_icon_path(appid)).scaled(
                            32, 32, Qt.KeepAspectRatio, Qt.SmoothTransformation
                        )
                        icon = QIcon(pixmap)
                        item.setIcon(icon)
                    item.setFlags(Qt.ItemIsEnabled)
                    item.setToolTip(f"{appid}")
                    if is_running:
                        item.setForeground(QColor("green"))
                    table.setItem(row, col, item)
                    col += 1

                    # GPU
                    gpu_combo = NoScrollComboBox()
                    gpu_combo.setEnabled(not is_running)  # Usar la subclase personalizada
                    gpu_combo.addItem(TRANSLATOR.translate("label.dgpu.auto"), None)
                    gpu_combo.setCurrentIndex(0)
                    for i, gpu in enumerate(HARDWARE_SERVICE.gpus):
                        gpu_combo.addItem(f"{gpu.value.capitalize()}", gpu)
                        if gpu == STEAM_SERVICE.get_prefered_gpu(appid):
                            gpu_combo.setCurrentIndex(i + 1)

                    gpu_combo.currentIndexChanged.connect(
                        lambda _, widget=gpu_combo, game=appid: self.__on_gpu_changed(  # pylint: disable=line-too-long
                            widget, game
                        )
                    )
                    table.setCellWidget(row, col, gpu_combo)
                    col += 1

                    # GPU
                    steamdeck_combo = NoScrollComboBox()
                    steamdeck_combo.setEnabled(not is_running)  # Usar la subclase personalizada
                    steamdeck_combo.addItem(TRANSLATOR.translate("label.steamdeck.no"), False)
                    steamdeck_combo.addItem(TRANSLATOR.translate("label.steamdeck.yes"), True)
                    steamdeck_combo.setCurrentIndex(1 if STEAM_SERVICE.is_steamdeck(appid) else 0)

                    steamdeck_combo.currentIndexChanged.connect(
                        lambda _, widget=steamdeck_combo, game=appid: self.__on_steamdeck_changed(  # pylint: disable=line-too-long
                            widget, game
                        )
                    )
                    table.setCellWidget(row, col, steamdeck_combo)
                    col += 1

                    # Metrics
                    if STEAM_SERVICE.metrics_enabled:
                        metrics_combo = NoScrollComboBox()  # Usar la subclase personalizada
                        metrics_combo.setEnabled(not is_running)
                        for level in MangoHudLevel:
                            metrics_combo.addItem(TRANSLATOR.translate(f"label.level.{level}"), level)
                            if level == STEAM_SERVICE.get_metrics_level(appid):
                                metrics_combo.setCurrentIndex(level.value)

                        metrics_combo.currentIndexChanged.connect(
                            lambda _, widget=metrics_combo, game=appid: self.__on_metrics_changed(  # pylint: disable=line-too-long
                                widget, game
                            )
                        )
                        table.setCellWidget(row, col, metrics_combo)
                        col += 1

                    sync_combo = NoScrollComboBox()  # Usar la subclase personalizada
                    for i, option in enumerate(WineSyncOption):
                        sync_combo.addItem(TRANSLATOR.translate(f"label.winesync.{option}"), option)
                        if option == STEAM_SERVICE.get_wine_sync(appid):
                            sync_combo.setCurrentIndex(i)

                    sync_combo.currentIndexChanged.connect(
                        lambda _, widget=sync_combo, game=appid: self.__on_winesync_changed(  # pylint: disable=line-too-long
                            widget, game
                        )
                    )
                    sync_combo.setEnabled(not is_running and STEAM_SERVICE.is_proton(appid))
                    table.setCellWidget(row, col, sync_combo)
                    col += 1

                    env_input = QLineEdit()
                    env_input.setText(STEAM_SERVICE.get_environment(appid))
                    env_input.setEnabled(not is_running)
                    env_input.textChanged.connect(lambda text, game=appid: self.__on_environment_changed(text, game))
                    table.setCellWidget(row, col, env_input)
                    col += 1

                    args_input = QLineEdit()
                    args_input.setText(STEAM_SERVICE.get_parameters(appid))
                    args_input.setEnabled(not is_running)
                    args_input.textChanged.connect(lambda text, game=appid: self.__on_params_changed(text, game))
                    table.setCellWidget(row, col, args_input)
                    col += 1

                    row += 1
                except Exception as e:
                    print(f"{e}")

            # Añadir la tabla al layout del contenedor
            container_layout.addWidget(table)

            # Añadir el área de scroll al layout principal
            layout.addWidget(scroll_area)

            EVENT_BUS.on(STEAM_SERVICE_CONNECTED, self.close)

    def __on_steamdeck_changed(self, widget, game: int):
        sd = widget.currentData()
        STEAM_SERVICE.set_steamdeck(sd, game)

    def __on_params_changed(self, text, game: int):
        STEAM_SERVICE.set_parameters(text, game)

    def __on_environment_changed(self, text, game: int):
        STEAM_SERVICE.set_environment(text, game)

    def __on_metrics_changed(self, widget, game: int):
        level = widget.currentData()
        STEAM_SERVICE.set_metrics_level(level, game)

    def __on_winesync_changed(self, widget, game: int):
        level = widget.currentData()
        STEAM_SERVICE.set_wine_sync(level, game)

    def __on_gpu_changed(self, widget, game: int):
        gpu_brand = widget.currentData()
        STEAM_SERVICE.set_prefered_gpu(gpu_brand, game)

    def show(self):
        """Override default show behaviour"""
        if GameList.INSTANCE == self:
            if self.__manage_parent:
                self.__parent.show()
            super().show()
            if self.__manage_parent:
                self.__parent.hide()

    def closeEvent(self, event):  # pylint: disable=invalid-name
        """Override default closeEvent behaviour"""
        if GameList.INSTANCE == self:
            if self.__manage_parent:
                self.__parent.closeEvent(event)
            super().closeEvent(event)
            GameList.INSTANCE = None
