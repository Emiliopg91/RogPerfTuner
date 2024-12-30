#!/bin/env python3
from lib import __version__
from lib.utils.single_instance import single_instance

single_instance.acquire()

from PyQt5.QtWidgets import QApplication
import sys

app = QApplication(sys.argv)

from lib.utils.constants import dev_mode

if dev_mode:
    print(f"Running in dev mode")
else:
    print(f"Running bundled file from '{sys.executable}'")

from lib.utils.logger import Logger

logger = Logger("Main")

logger.info("###################################################")
logger.info("#            Starting RogControlCenter            #")
logger.info("###################################################")
logger.info(f"Version {__version__}")
logger.info("Starting initialization")
logger.addTab()


if not dev_mode:
    from lib.utils.application import application

    application.create_menu_entry()
    application.enable_autostart()

from lib.gui.password_dialog import password_dialog
from lib.utils.configuration import configuration

if configuration.settings.password is None:
    password_dialog.show()

from lib.server.dbus_server import dbus_server
from lib.gui.tray_icon import tray_icon

tray_icon.show()

dbus_server.start()

from lib.utils.autoupdater import auto_updater

auto_updater.start()

logger.remTab()
logger.info("Application ready")

sys.exit(app.exec_())
