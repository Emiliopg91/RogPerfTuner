#!/bin/env python3

# pylint: disable=C0415, C0413, C0412, C0411

import asyncio
from qasync import QEventLoop

import setproctitle
from rcc import __app_name__, __version__

setproctitle.setproctitle(__app_name__)

from PyQt5.QtWidgets import QApplication
import sys


def get_application_lock():
    """Get the lock"""
    from rcc.utils.single_instance import SINGLE_INSTANCE

    SINGLE_INSTANCE.acquire()


def create_qt_application() -> QApplication:
    """Initialize QApplication"""
    q_app = QApplication(sys.argv)
    q_loop = QEventLoop(q_app)
    asyncio.set_event_loop(q_loop)
    return q_app, q_loop


def initialize_application():  # pylint:disable=R0914
    """Application startup"""
    import os
    from rcc.utils.constants import DEV_MODE, USER_UPDATE_FOLDER, LOG_FILE, LOG_FOLDER, LOG_OLD_FOLDER

    from framework.logger import Logger

    Logger.initialize(LOG_FILE, LOG_FOLDER, LOG_OLD_FOLDER)
    logger = Logger("Main")

    if DEV_MODE:
        print("Running in dev mode")
    else:
        print(f"Running bundled file from '{os.getenv("APPIMAGE")}'")

    logger.info("###################################################")
    logger.info("#            Starting RogControlCenter            #")
    logger.info("###################################################")
    logger.info(f"Version {__version__}")
    logger.info("Starting initialization")
    logger.add_tab()

    from rcc.gui.notifier import NOTIFIER
    from rcc.utils.beans import TRANSLATOR

    NOTIFIER.show_toast(TRANSLATOR.translate("initializing"))

    """Create application files"""
    if not DEV_MODE:
        from rcc.services.application_service import APPLICATION_SERVICE

        APPLICATION_SERVICE.generate_run()
        APPLICATION_SERVICE.create_menu_entry()
        APPLICATION_SERVICE.enable_autostart()

    """Ask for sudo password"""
    from rcc.gui.password_dialog import PASSWORD_DIALOG
    from rcc.utils.configuration import CONFIGURATION

    if CONFIGURATION.settings.password is None:
        PASSWORD_DIALOG.show()

    from rcc.services.openrgb_service import OPEN_RGB_SERVICE  # pylint: disable=W0611

    """Install Decky plugin"""
    from rcc.services.games_service import GAME_SERVICE

    GAME_SERVICE.install_rccdc()

    """Show tray icon"""
    from rcc.gui.tray_icon import TRAY_ICON

    TRAY_ICON.show()

    from rcc.services.platform_service import PLATFORM_SERVICE

    if len(GAME_SERVICE.running_games) == 0:
        PLATFORM_SERVICE.restore_profile()

    """Start dbus server"""
    from rcc.communications.server.dbus_server import DBUS_SERVER

    DBUS_SERVER.start()

    """Start autoupdater"""
    from framework.autoupdater import AutoUpdater
    from rcc.services.application_service import APPLICATION_SERVICE

    auto_updater = AutoUpdater(
        __app_name__,
        __version__,
        "Emiliopg91",
        "RogControlCenter",
        USER_UPDATE_FOLDER,
        APPLICATION_SERVICE.relaunch_application,
        DEV_MODE,
    )

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
