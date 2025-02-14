import sys
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QTableWidget, QTableWidgetItem
from PyQt5.QtCore import Qt, QRect, QTimer
from PyQt5.QtGui import QGuiApplication


class TransparentBar(QWidget):
    def __init__(self):
        super().__init__()

        # Configurar la ventana sin bordes y siempre encima
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.setAttribute(Qt.WA_TranslucentBackground)  # Fondo transparente

        # Hacer la ventana semitransparente
        self.setWindowOpacity(0.8)

        # Obtener la pantalla con mayor resolución
        screens = QGuiApplication.screens()
        largest_screen = max(screens, key=lambda s: s.geometry().width() * s.geometry().height())
        screen_geometry = largest_screen.geometry()
        screen_width = screen_geometry.width()

        # Establecer el tamaño de la ventana
        self.setGeometry(QRect(screen_geometry.x(), screen_geometry.y(), 300, 220))

        labels = ["CPU %", "CPU º", "GPU %", "GPU º", "FPS"]

        # Configurar el layout y la tabla para mostrar los datos
        self.layout = QVBoxLayout()
        self.table = QTableWidget(len(labels), 2, self)  # 4 filas, 2 columnas
        self.table.setHorizontalHeaderLabels(["", ""])  # Sin cabeceras
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setVisible(False)

        # Datos iniciales de la tabla
        for i, label in enumerate(labels):
            self.table.setItem(i, 0, QTableWidgetItem(label))
            self.table.setItem(i, 1, QTableWidgetItem("N/A"))

        self.layout.addWidget(self.table)
        self.setLayout(self.layout)

        # Configurar el temporizador para actualizar la tabla cada segundo
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_table)
        self.timer.start(1000)

        # Mostrar la ventana
        self.show()

    def update_table(self):
        for i in range(5):
            self.table.setItem(i, 1, QTableWidgetItem("N/A"))


if __name__ == "__main__":
    app = QApplication(sys.argv)
    bar = TransparentBar()
    sys.exit(app.exec_())
