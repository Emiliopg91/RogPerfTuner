from ..utils.configuration import configuration
from ..utils.cryptography import cryptography
from ..utils.logger import Logger
from ..utils.translator import translator

from PyQt5.QtGui import QCursor
from PyQt5.QtWidgets import (
    QApplication,
    QDialog,
    QVBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QMessageBox,
)

import subprocess
import sys


class PasswordDialog(QDialog):
    def __init__(self):
        self.logger = Logger(self.__class__.__name__)

        super().__init__()
        self.setWindowTitle(translator.translate("authentication.required"))
        self.setGeometry(100, 100, 300, 150)

        # Layout principal
        layout = QVBoxLayout()

        # Etiqueta de instrucción
        self.label = QLabel(f"{translator.translate("enter.sudo.password")}:")
        layout.addWidget(self.label)

        # Campo de entrada de contraseña
        self.password_input = QLineEdit()
        self.password_input.setEchoMode(QLineEdit.Password)
        layout.addWidget(self.password_input)

        # Botón de aceptar
        self.ok_button = QPushButton(translator.translate("accept"))
        self.ok_button.clicked.connect(self.on_accept)
        layout.addWidget(self.ok_button)

        # Botón de cancelar
        self.cancel_button = QPushButton(translator.translate("cancel"))
        self.cancel_button.clicked.connect(self.reject)
        layout.addWidget(self.cancel_button)

        self.setLayout(layout)

        self.center_window_on_current_screen()

    def on_accept(self):
        self.ok_button.setDisabled(True)
        self.cancel_button.setDisabled(True)

        if self.check_password(self.password_input.text()):
            configuration.config.settings.password = cryptography.encrypt_string(
                self.password_input.text()
            )
            configuration.save_config()
            self.accept()
        else:
            self.ok_button.setDisabled(False)
            self.cancel_button.setDisabled(False)
            QMessageBox.warning(
                None, "Error", translator.translate("authentication.failed")
            )

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

    def check_password(self, password: str) -> bool:
        self.logger.info("Checking password")
        try:
            process = subprocess.run(
                ["sudo", "-S", "echo", "Password correct"],
                input=password + "\n",
                text=True,
                capture_output=True,
            )
            if process.returncode == 0:
                self.logger.info("Authentication successful")
                return True
            else:
                self.logger.error("Authentication failed")
                return False
        except Exception as e:
            self.logger.error(f"Error during authentication: {e}")
            return False

    def show(self):
        if self.exec_() != QDialog.Accepted:
            QMessageBox.information(
                None,
                translator.translate("canceled"),
                translator.translate("user.canceled.operation"),
            )
            sys.exit()


password_dialog = PasswordDialog()
