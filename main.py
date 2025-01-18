#!/bin/env python3

# pylint: disable=C0415, C0413, C0412, C0411, E0611

import asyncio
from qasync import QEventLoop

import setproctitle
from rcc import __app_name__, __version__

setproctitle.setproctitle(__app_name__)

from PyQt5.QtWidgets import QApplication
import sys


def get_application_lock():
    """Get the lock"""
    from rcc.utils.single_instance import single_instance

    single_instance.acquire()


def create_qt_application() -> QApplication:
    """Initialize QApplication"""
    q_app = QApplication(sys.argv)
    q_loop = QEventLoop(q_app)
    asyncio.set_event_loop(q_loop)
    return q_app, q_loop


def initialize_application():
    """Application startup"""
    import os
    from rcc.utils.constants import dev_mode

    from rcc.utils.logger import Logger

    logger = Logger("Main")

    if dev_mode:
        print("Running in dev mode")
    else:
        print(f"Running bundled file from '{os.getenv("APPIMAGE")}'")

    logger.info("###################################################")
    logger.info("#            Starting RogControlCenter            #")
    logger.info("###################################################")
    logger.info(f"Version {__version__}")
    logger.info("Starting initialization")
    logger.add_tab()

    from rcc.gui.notifier import notifier
    from rcc.utils.translator import translator

    notifier.show_toast(translator.translate("initializing"))

    """Create application files"""
    if not dev_mode:
        from rcc.utils.application import application

        application.generate_run()
        application.create_menu_entry()
        application.enable_autostart()

    """Ask for sudo password"""
    from rcc.gui.password_dialog import password_dialog
    from rcc.utils.configuration import configuration

    if configuration.settings.password is None:
        password_dialog.show()

    from rcc.services.openrgb_service import open_rgb_service  # pylint: disable=W0611

    """Install Decky plugin"""
    from rcc.services.games_service import games_service

    games_service.install_rccdc()

    """Show tray icon"""
    from rcc.gui.tray_icon import tray_icon

    tray_icon.show()

    """Start dbus server"""
    from rcc.server.dbus_server import dbus_server

    dbus_server.start()

    """Start autoupdater"""
    from rcc.utils.autoupdater import auto_updater

    auto_updater.start()

    logger.rem_tab()
    logger.info("Application ready")


def start_qt_application(application: QApplication):
    """Launch event loop"""
    application.setQuitOnLastWindowClosed(False)
    sys.exit(application.exec())


if __name__ == "__main__":
    get_application_lock()

    app, loop = create_qt_application()

    initialize_application()

    with loop:
        loop.run_forever()
