from ..utils.configuration import configuration
from ..utils.cryptography import cryptography

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
        super().__init__()
        self.attempts = 0
        self.setWindowTitle("Autenticación requerida")
        self.setGeometry(100, 100, 300, 150)

        # Layout principal
        layout = QVBoxLayout()

        # Etiqueta de instrucción
        self.label = QLabel("Introduce tu contraseña:")
        layout.addWidget(self.label)

        # Campo de entrada de contraseña
        self.password_input = QLineEdit()
        self.password_input.setEchoMode(QLineEdit.Password)
        layout.addWidget(self.password_input)

        # Botón de aceptar
        self.ok_button = QPushButton("Aceptar")
        self.ok_button.clicked.connect(self.on_accept)
        layout.addWidget(self.ok_button)

        # Botón de cancelar
        self.cancel_button = QPushButton("Cancelar")
        self.cancel_button.clicked.connect(self.reject)
        layout.addWidget(self.cancel_button)

        self.setLayout(layout)

        self.center_window_on_current_screen()

    def on_accept(self):
        self.ok_button.setDisabled(True)
        self.cancel_button.setDisabled(True)

        self.attempts += 1
        if self.check_password(self.password_input.text()):
            configuration.config.settings.password = cryptography.encrypt_string(
                self.password_input.text()
            )
            configuration.save_config()
            self.accept()
        else:
            if self.attempts < 3:
                self.ok_button.setDisabled(False)
                self.cancel_button.setDisabled(False)
                QMessageBox.warning(None, "Error", "authentication.failed")
            else:
                QMessageBox.warning(None, "Error", "max.failed.attemps")
                sys.exit(1)

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
        print("Checking password")
        try:
            # Ejecutar el comando con la contraseña usando sudo
            process = subprocess.run(
                ["sudo", "-S", "echo", "Password correct"],
                input=password + "\n",
                text=True,
                capture_output=True,
            )
            if process.returncode == 0:
                print("Authentication successful")
                return True
            else:
                print("Authentication failed")
                return False
        except Exception as e:
            self.logger.error(f"Error during authentication: {e}")
            return False

    def show(self):
        if self.exec_() != QDialog.Accepted:
            QMessageBox.information(
                None, "Cancelado", "El usuario canceló la operación."
            )
            sys.exit()


password_dialog = PasswordDialog()
