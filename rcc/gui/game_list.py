# pylint: disable=E0611
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

from rcc.utils.constants import icons_path
from rcc.models.platform_profile import PlatformProfile
from rcc.services.games_service import games_service
from rcc.utils.gui_utils import NoScrollComboBox
from rcc.utils.translator import translator


class GameList(QDialog):
    """Window for game list"""

    def __init__(self, parent: QWidget, manage_parent=False):  # pylint: disable=R0914
        super().__init__(parent)
        self.__parent = parent
        self.__manage_parent = manage_parent
        self.setWindowTitle(translator.translate("game.performance.configuration"))
        self.setFixedSize(700, 500)
        self.setWindowIcon(QIcon(f"{icons_path}/icon-45x45.png"))
        self.setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose)

        # Configurar layout principal
        layout = QVBoxLayout(self)

        # Crear el área de scroll
        scroll_area = QScrollArea(self)
        scroll_area.setWidgetResizable(True)
        scroll_area.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)

        # Crear un widget contenedor
        container = QWidget()
        scroll_area.setWidget(container)

        # Layout del contenedor
        container_layout = QVBoxLayout(container)

        # Crear la tabla
        game_cfg = games_service.get_games()
        games = list(game_cfg.keys())

        table = QTableWidget(len(games), 2)  # 10 filas, 2 columnas
        table.setSizeAdjustPolicy(QAbstractScrollArea.SizeAdjustPolicy.AdjustToContents)
        table.setHorizontalHeaderLabels([translator.translate("game.title"), translator.translate("profile")])
        table.verticalHeader().setVisible(False)

        # Configurar el ancho de las columnas
        header = table.horizontalHeader()
        header.setSectionResizeMode(0, QHeaderView.ResizeMode.Stretch)  # Primera columna se expande
        header.setSectionResizeMode(
            1, QHeaderView.ResizeMode.ResizeToContents
        )  # Segunda columna ajusta su tamaño al contenido
        header.setHighlightSections(False)  # Evitar que el encabezado se resalte

        # Desactivar edición y selección en la tabla
        table.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
        table.setSelectionMode(QAbstractItemView.SelectionMode.NoSelection)
        table.setFocusPolicy(Qt.NoFocus)

        # Llenar la tabla con datos
        i = 0
        for game in games:
            # Primera columna: texto
            item = QTableWidgetItem(game_cfg[game].name)
            item.setFlags(Qt.ItemIsEnabled)  # Hacer el ítem no editable
            table.setItem(i, 0, item)

            # Segunda columna: dropdown
            combo = NoScrollComboBox()  # Usar la subclase personalizada
            for profile in PlatformProfile:
                combo.addItem(translator.translate(f"label.profile.{profile.name}"), profile)

            default_index = combo.findData(
                PlatformProfile(game_cfg[game].profile)
            )  # Buscar el índice basado en el dato asociado
            if default_index != -1:
                combo.setCurrentIndex(default_index)

            # Conectar el evento currentIndexChanged al manejador
            combo.currentIndexChanged.connect(
                lambda _, widget=combo, game_id=game: self.__on_profile_changed(widget, game_id)
            )
            table.setCellWidget(i, 1, combo)

            i += 1

        # Añadir la tabla al layout del contenedor
        container_layout.addWidget(table)

        # Añadir el área de scroll al layout principal
        layout.addWidget(scroll_area)

    def __on_profile_changed(self, widget, game_id):
        selected_profile = widget.currentData()  # Obtiene el objeto asociado al elemento seleccionado
        games_service.set_game_profile(game_id, selected_profile)

    def show(self):
        """Override default show behaviour"""
        if self.__manage_parent:
            self.__parent.show()
        super().show()
        if self.__manage_parent:
            self.__parent.hide()

    def closeEvent(self, event):  # pylint: disable=C0103
        """Override default closeEvent behaviour"""
        if self.__manage_parent:
            self.__parent.closeEvent(event)
        super().closeEvent(event)
