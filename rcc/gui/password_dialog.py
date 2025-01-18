import subprocess
import sys

# pylint: disable=E0611, E0401
from PyQt5.QtWidgets import (
    QDialog,
    QVBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QMessageBox,
)

from rcc.utils.configuration import configuration
from rcc.utils.cryptography import cryptography
from rcc.utils.gui_utils import GuiUtils
from rcc.utils.logger import Logger
from rcc.utils.translator import translator


class PasswordDialog(QDialog):
    """Sudo password dialog"""

    def __init__(self):
        self._logger = Logger()

        super().__init__()
        self.setWindowTitle(translator.translate("authentication.required"))
        self.setGeometry(100, 100, 300, 150)

        # Layout principal
        layout = QVBoxLayout()

        # Etiqueta de instrucción
        self._label = QLabel(f"{translator.translate("enter.sudo.password")}:")
        layout.addWidget(self._label)

        # Campo de entrada de contraseña
        self._password_input = QLineEdit()
        self._password_input.setEchoMode(QLineEdit.Password)
        layout.addWidget(self._password_input)

        # Botón de aceptar
        self._ok_button = QPushButton(translator.translate("accept"))
        self._ok_button.clicked.connect(self.on_accept)
        layout.addWidget(self._ok_button)

        # Botón de cancelar
        self._cancel_button = QPushButton(translator.translate("cancel"))
        self._cancel_button.clicked.connect(self.reject)
        layout.addWidget(self._cancel_button)

        self.setLayout(layout)

        GuiUtils.center_window_on_current_screen(self)

    def on_accept(self):
        """Accept event handler"""
        self._ok_button.setDisabled(True)
        self._cancel_button.setDisabled(True)

        if self.check_password(self._password_input.text()):
            configuration.settings.password = cryptography.encrypt_string(self._password_input.text())
            configuration.save_config()
            self.accept()
        else:
            self._ok_button.setDisabled(False)
            self._cancel_button.setDisabled(False)
            QMessageBox.warning(None, "Error", translator.translate("authentication.failed"))

    def check_password(self, password: str) -> bool:
        """check current password"""
        self._logger.info("Checking password")
        try:
            process = subprocess.run(
                ["sudo", "-S", "echo", "Password correct"],
                input=password + "\n",
                text=True,
                capture_output=True,
                check=False,
            )
            if process.returncode == 0:
                self._logger.info("Authentication successful")
                return True

            self._logger.error("Authentication failed")
            return False
        except Exception as e:
            self._logger.error(f"Error during authentication: {e}")
            return False

    def show(self):
        """Show dialog"""
        if self.exec_() != QDialog.Accepted:
            QMessageBox.information(
                None,
                translator.translate("canceled"),
                translator.translate("user.canceled.operation"),
            )
            sys.exit()


password_dialog = PasswordDialog()
