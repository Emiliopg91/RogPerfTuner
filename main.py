#!/bin/env python3
from lib.utils.single_instance import single_instance
single_instance.acquire()

from PyQt5.QtWidgets import (
    QApplication
)
import sys

app = QApplication(sys.argv)

from lib.utils.constants import dev_mode
if dev_mode:
    print(f"Running in dev mode")
else:
    print(f"Running bundled file from '{sys.executable}'")

from lib.utils.logger import Logger
logger = Logger("Main")

from lib.utils.event_bus import event_bus

import time
import os
import signal

logger.info("###################################################")
logger.info("#            Starting RogControlCenter            #")
logger.info("###################################################")
logger.info("Starting initialization")
logger.addTab()

from lib.server.dbus_server import dbus_server
from lib.gui.tray_icon import tray_icon
from lib.gui.password_dialog import password_dialog
from lib.utils.configuration import configuration

from lib.clients.openrgb.openrgb_client import open_rgb_client

if __name__ == "__main__":
    if configuration.settings.password is None:
        password_dialog.show()

    tray_icon.show()
    
    dbus_server.start()

    logger.remTab()
    logger.info("Application ready")
    
    sys.exit(app.exec_())