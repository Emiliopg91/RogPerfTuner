#!/bin/env python3

# pylint: disable=import-outside-toplevel, wrong-import-position, ungrouped-imports, wrong-import-order

import asyncio
from qasync import QEventLoop

import setproctitle
from rcc.utils.constants import APP_NAME, VERSION

setproctitle.setproctitle(f"{APP_NAME} v{VERSION}")

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


def initialize_application():  # pylint:disable=too-many-locals
    """Application startup"""
    import os
    from rcc.utils.constants import DEV_MODE, USER_UPDATE_FOLDER, LOG_FILE, USER_LOG_FOLDER, LOG_OLD_FOLDER

    from framework.logger import Logger

    Logger.initialize(LOG_FILE, USER_LOG_FOLDER, LOG_OLD_FOLDER)
    logger = Logger("Main")

    if DEV_MODE:
        print("Running in dev mode")
    else:
        print(f"Running bundled file from '{os.getenv("APPIMAGE")}'")

    logger.info("###################################################")
    logger.info("#            Starting RogControlCenter            #")
    logger.info("###################################################")
    logger.info(f"Version {VERSION}")
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

    """Ask for sudo password"""
    from rcc.gui.password_dialog import PASSWORD_DIALOG
    from rcc.utils.configuration import CONFIGURATION

    if CONFIGURATION.settings.password is None:
        PASSWORD_DIALOG.show()

    from rcc.services.rgb_service import RGB_SERVICE  # pylint: disable=unused-import

    """Install Decky plugin"""
    from rcc.services.steam_service import STEAM_SERVICE

    STEAM_SERVICE.install_rccdc()

    """Show tray icon"""
    from rcc.gui.tray_icon import TRAY_ICON

    TRAY_ICON.show()

    from rcc.services.hardware_service import HARDWARE_SERVICE

    HARDWARE_SERVICE.set_panel_overdrive(False)

    from rcc.services.profile_service import PROFILE_SERVICE

    if len(STEAM_SERVICE.running_games) == 0:
        PROFILE_SERVICE.restore_profile()

    """Start rest server"""
    from rcc.communications.server.rest_server import REST_SERVER

    REST_SERVER.start()

    """Start autoupdater"""
    from framework.autoupdater import AutoUpdater
    from rcc.services.application_service import APPLICATION_SERVICE

    auto_updater = AutoUpdater(
        APP_NAME,
        VERSION,
        "Emiliopg91",
        "RogControlCenter",
        USER_UPDATE_FOLDER,
        APPLICATION_SERVICE.relaunch_application,
        DEV_MODE,
        lambda: len(STEAM_SERVICE.running_games.keys()) == 0,
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
