from .notifier import notifier
from ..utils.event_bus import event_bus
from ..utils.logger import Logger
from ..utils.translator import translator

from PyQt5.QtGui import QCursor
from PyQt5.QtWidgets import (
    QApplication,
    QDialog,
    QVBoxLayout,
    QLabel,
    QPushButton,
)

import os
import signal


class ApplyUpdate(QDialog):
    def __init__(self):
        self.logger = Logger(self.__class__.__name__)

        super().__init__()
        self.setWindowTitle(translator.translate("update.available"))
        self.setGeometry(100, 100, 300, 150)

        # Layout principal
        layout = QVBoxLayout()

        # Etiqueta de instrucción
        self.label = QLabel(f"{translator.translate("apply.now")}:")
        layout.addWidget(self.label)

        # Botón de aceptar
        self.ok_button = QPushButton(translator.translate("accept"))
        self.ok_button.clicked.connect(self.accept)
        layout.addWidget(self.ok_button)

        # Botón de cancelar
        self.cancel_button = QPushButton(translator.translate("cancel"))
        self.cancel_button.clicked.connect(self.reject)
        layout.addWidget(self.cancel_button)

        self.setLayout(layout)

        self.center_window_on_current_screen()

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

    def show(self):
        if self.exec_() == QDialog.Accepted:
            notifier.show_toast(translator.translate("applying.update"))
            event_bus.emit("stop", None)
            event_bus.emit("update", None)
            os.kill(os.getpid(), signal.SIGKILL)


apply_update = ApplyUpdate()
