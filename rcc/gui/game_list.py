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
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QIcon

from rcc.communications.client.websocket.steam.steam_client import SteamGameDetails, STEAM_CLIENT
from rcc.models.mangohud_level import MangoHudLevel
from rcc.models.ntsync_option import NtSyncOption
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
    ):  # pylint: disable=too-many-locals, too-many-branches
        super().__init__(parent)
        if GameList.INSTANCE is None:  # pylint: disable=too-many-nested-blocks
            GameList.INSTANCE = self
            self.__parent = parent
            self.__manage_parent = manage_parent
            self.setWindowTitle(TRANSLATOR.translate("game.performance.configuration"))
            self.setFixedSize(600, 600)
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

            columns = [
                TRANSLATOR.translate("game.title"),
                TRANSLATOR.translate("used.gpu"),
            ]
            if STEAM_SERVICE.metrics_enabled:
                columns.append(TRANSLATOR.translate("metrics"))
            if HARDWARE_SERVICE.is_ntsync_ready:
                columns.append(TRANSLATOR.translate("ntsync"))

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

            # Llenar la tabla con datos
            row = 0
            game_details = STEAM_CLIENT.get_apps_details(*appids)  # pylint: disable=unused-variable
            game_details = sorted(game_details, key=lambda item: item.name)
            for game in game_details:
                try:
                    col = 0
                    # Title
                    item = QTableWidgetItem(game.name)
                    item.setFlags(Qt.ItemIsEnabled)
                    item.setToolTip(f"{game.appid}")
                    table.setItem(row, col, item)
                    col += 1

                    # GPU
                    gpu_combo = NoScrollComboBox()
                    gpu_combo.setEnabled(app_id is None or app_id == game.appid)  # Usar la subclase personalizada
                    gpu_combo.addItem(TRANSLATOR.translate("label.dgpu.auto"), None)
                    gpu_combo.setCurrentIndex(0)
                    for i, gpu in enumerate(HARDWARE_SERVICE.gpus):
                        gpu_combo.addItem(f"{gpu.value.capitalize()}", gpu)
                        if gpu == STEAM_SERVICE.get_prefered_gpu(game.appid):
                            gpu_combo.setCurrentIndex(i + 1)

                    gpu_combo.currentIndexChanged.connect(
                        lambda _, widget=gpu_combo, game=game: self.__on_gpu_changed(  # pylint: disable=line-too-long
                            widget, game
                        )
                    )
                    table.setCellWidget(row, col, gpu_combo)
                    col += 1

                    # Metrics
                    if STEAM_SERVICE.metrics_enabled:
                        metrics_combo = NoScrollComboBox()  # Usar la subclase personalizada
                        metrics_combo.setEnabled(app_id is None or app_id == game.appid)
                        for level in MangoHudLevel:
                            metrics_combo.addItem(TRANSLATOR.translate(f"label.level.{level}"), level)
                            if level == STEAM_SERVICE.get_metrics_level(game.appid):
                                metrics_combo.setCurrentIndex(level.value)

                        metrics_combo.currentIndexChanged.connect(
                            lambda _, widget=metrics_combo, game=game: self.__on_metrics_changed(  # pylint: disable=line-too-long
                                widget, game
                            )
                        )
                        table.setCellWidget(row, col, metrics_combo)
                        col += 1

                    if HARDWARE_SERVICE.is_ntsync_ready:
                        ntsync_combo = NoScrollComboBox()  # Usar la subclase personalizada
                        ntsync_combo.setEnabled(app_id is None or app_id == game.appid)
                        for option in NtSyncOption:
                            ntsync_combo.addItem(TRANSLATOR.translate(f"label.ntsync.{option}"), option)
                            if option == STEAM_SERVICE.get_ntsync_level(game.appid):
                                ntsync_combo.setCurrentIndex(option.value)

                        ntsync_combo.currentIndexChanged.connect(
                            lambda _, widget=ntsync_combo, game=game: self.__on_ntsync_changed(  # pylint: disable=line-too-long
                                widget, game
                            )
                        )
                        table.setCellWidget(row, col, ntsync_combo)
                        col += 1

                    row += 1
                except Exception as e:
                    print(f"{e}")

            # Añadir la tabla al layout del contenedor
            container_layout.addWidget(table)

            # Añadir el área de scroll al layout principal
            layout.addWidget(scroll_area)

            EVENT_BUS.on(STEAM_SERVICE_CONNECTED, self.close)

    def __on_metrics_changed(self, widget, game: SteamGameDetails):
        level = widget.currentData()
        game.launch_opts = STEAM_SERVICE.set_metrics_level(level, game.appid, game.launch_opts)

    def __on_ntsync_changed(self, widget, game: SteamGameDetails):
        level = widget.currentData()
        game.launch_opts = STEAM_SERVICE.set_ntsync_level(level, game.appid, game.launch_opts)

    def __on_gpu_changed(self, widget, game: SteamGameDetails):
        gpu_brand = widget.currentData()
        game.launch_opts = STEAM_SERVICE.set_prefered_gpu(gpu_brand, game.appid, game.launch_opts)

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
