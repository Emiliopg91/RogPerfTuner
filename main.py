#!/bin/env python3

# pylint: disable=C0413, C0412, C0411, E0611

from lib import __version__
from lib.utils.single_instance import single_instance

single_instance.acquire()

import sys

from PyQt5.QtWidgets import QApplication

app = QApplication(sys.argv)

from datetime import datetime

from lib.utils.constants import dev_mode

start_time = datetime.now()

if dev_mode:
    print("Running in dev mode")
else:
    print(f"Running bundled file from '{sys.executable}'")

from lib.utils.logger import Logger

logger = Logger("Main")

logger.info("###################################################")
logger.info("#            Starting RogControlCenter            #")
logger.info("###################################################")
logger.info(f"Version {__version__}")
logger.info("Starting initialization")
logger.add_tab()


from lib.gui.notifier import notifier
from lib.utils.translator import translator

notifier.show_toast(translator.translate("initializing"))

if not dev_mode:
    from lib.utils.application import application

    application.generate_run()
    application.create_menu_entry()
    application.enable_autostart()

from lib.gui.password_dialog import password_dialog
from lib.utils.configuration import configuration

if configuration.settings.password is None:
    password_dialog.show()

from lib.gui.tray_icon import tray_icon
from lib.server.dbus_server import dbus_server

tray_icon.show()

dbus_server.start()

from lib.utils.autoupdater import auto_updater

auto_updater.start()

logger.rem_tab()
logger.info("Application ready")

sys.exit(app.exec_())
