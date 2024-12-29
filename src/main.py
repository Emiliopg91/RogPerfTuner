#!/bin/env python3
from utils.single_instance import single_instance

single_instance.acquire()

from PyQt5.QtWidgets import QApplication
import sys

app = QApplication(sys.argv)

from utils.constants import dev_mode

if dev_mode:
    print(f"Running in dev mode")
else:
    print(f"Running bundled file from '{sys.executable}'")

from utils.logger import Logger

logger = Logger("Main")

logger.info("###################################################")
logger.info("#            Starting RogControlCenter            #")
logger.info("###################################################")
logger.info("Starting initialization")
logger.addTab()

from server.dbus_server import dbus_server
from gui.tray_icon import tray_icon
from gui.password_dialog import password_dialog
from utils.configuration import configuration

if __name__ == "__main__":
    if configuration.settings.password is None:
        password_dialog.show()

    tray_icon.show()

    dbus_server.start()

    logger.remTab()
    logger.info("Application ready")

    sys.exit(app.exec_())
