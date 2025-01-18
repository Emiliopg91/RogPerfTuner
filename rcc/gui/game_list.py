# pylint: disable=E0611
from PyQt5.QtWidgets import (
    QDialog,
    QVBoxLayout,
    QTableWidget,
    QTableWidgetItem,
    QComboBox,
    QScrollArea,
    QAbstractScrollArea,
    QHeaderView,
    QWidget,
    QAbstractItemView,
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QWheelEvent, QIcon

from rcc.utils.constants import icons_path
from rcc.models.thermal_throttle_profile import ThermalThrottleProfile
from rcc.services.games_service import games_service
from rcc.utils.translator import translator


class NoScrollComboBox(QComboBox):
    """Implementation without wheel event"""

    def wheelEvent(self, event: QWheelEvent):  # pylint: disable=C0103
        """Ignore wheel event"""
        event.ignore()


class GameList(QDialog):
    """Window for game list"""

    def __init__(self, parent: QWidget = None):  # pylint: disable=R0914
        if parent is not None:
            super().__init__(parent)
        else:
            super().__init__()
        self.setWindowTitle(translator.translate("game.performance.configuration"))
        self.setFixedSize(700, 500)
        self.setWindowIcon(QIcon(f"{icons_path}/icon-45x45.png"))

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
        game_cfg = games_service.get_games()
        games = list(game_cfg.keys())

        table = QTableWidget(len(games), 2)  # 10 filas, 2 columnas
        table.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)
        table.setHorizontalHeaderLabels([translator.translate("game.title"), translator.translate("profile")])
        table.verticalHeader().setVisible(False)

        # Configurar el ancho de las columnas
        header = table.horizontalHeader()
        header.setSectionResizeMode(0, QHeaderView.Stretch)  # Primera columna se expande
        header.setSectionResizeMode(1, QHeaderView.ResizeToContents)  # Segunda columna ajusta su tamaño al contenido
        header.setHighlightSections(False)  # Evitar que el encabezado se resalte

        # Desactivar edición y selección en la tabla
        table.setEditTriggers(QAbstractItemView.NoEditTriggers)
        table.setSelectionMode(QAbstractItemView.NoSelection)
        table.setFocusPolicy(Qt.NoFocus)

        # Llenar la tabla con datos
        for i, game in enumerate(games):
            # Primera columna: texto
            item = QTableWidgetItem(game)
            item.setFlags(Qt.ItemIsEnabled)  # Hacer el ítem no editable
            table.setItem(i, 0, item)

            # Segunda columna: dropdown
            combo = NoScrollComboBox()  # Usar la subclase personalizada
            for profile in ThermalThrottleProfile:
                combo.addItem(translator.translate(f"label.profile.{profile.name}"), profile)

            default_index = combo.findData(
                ThermalThrottleProfile(game_cfg[game].profile)
            )  # Buscar el índice basado en el dato asociado
            if default_index != -1:
                combo.setCurrentIndex(default_index)

            # Conectar el evento currentIndexChanged al manejador
            combo.currentIndexChanged.connect(lambda index, row=i, widget=combo: self.on_profile_changed(row, widget))
            table.setCellWidget(i, 1, combo)

        # Añadir la tabla al layout del contenedor
        container_layout.addWidget(table)

        # Añadir el área de scroll al layout principal
        layout.addWidget(scroll_area)

    def on_profile_changed(self, row, widget):
        """
        Manejar el cambio de perfil en el dropdown.

        :param row: Índice de la fila del juego.
        :param widget: El QComboBox donde ocurrió el cambio.
        """
        selected_profile = widget.currentData()  # Obtiene el objeto asociado al elemento seleccionado
        game_name = self.findChild(QTableWidget).item(row, 0).text()  # Obtiene el nombre del juego
        games_service.set_game_profile(game_name, selected_profile)
