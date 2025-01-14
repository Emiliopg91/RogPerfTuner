import sys
from PyQt5.QtWidgets import (
    QApplication,
    QDialog,
    QVBoxLayout,
    QTableWidget,
    QTableWidgetItem,
    QComboBox,
    QScrollArea,
    QAbstractScrollArea,
    QWidget,
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QWheelEvent


class NoScrollComboBox(QComboBox):
    def wheelEvent(self, event: QWheelEvent):
        # Ignorar el evento de la rueda para evitar cambiar el valor
        event.ignore()


class PopupWindow(QDialog):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Ventana Emergente")
        self.setFixedSize(500, 400)  # Tamaño fijo de la ventana

        # Configurar layout principal
        layout = QVBoxLayout(self)

        # Crear el área de scroll
        scroll_area = QScrollArea(self)
        scroll_area.setWidgetResizable(True)

        # Crear un widget contenedor
        container = QWidget()
        scroll_area.setWidget(container)

        # Layout del contenedor
        container_layout = QVBoxLayout(container)

        # Crear la tabla
        table = QTableWidget(10, 2)  # 10 filas, 2 columnas
        table.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)
        table.setHorizontalHeaderLabels(["Texto", "Opciones"])
        table.verticalHeader().setVisible(False)

        # Llenar la tabla con datos
        for i in range(10):
            # Primera columna: texto
            table.setItem(i, 0, QTableWidgetItem(f"Texto {i + 1}"))

            # Segunda columna: dropdown
            combo = NoScrollComboBox()  # Usar la subclase personalizada
            combo.addItems(["Opción 1", "Opción 2", "Opción 3"])
            table.setCellWidget(i, 1, combo)

        # Añadir la tabla al layout del contenedor
        container_layout.addWidget(table)

        # Añadir el área de scroll al layout principal
        layout.addWidget(scroll_area)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = PopupWindow()
    window.exec_()
