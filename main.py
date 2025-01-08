#!/bin/env python3

# pylint: disable=C0415, C0413, C0412, C0411, E0611

from PyQt5.QtWidgets import QApplication
import sys


def get_application_lock():
    """Get the lock"""
    from lib.utils.single_instance import single_instance

    single_instance.acquire()


def create_qt_application() -> QApplication:
    """Initialize QApplication"""
    return QApplication(sys.argv)


def initialize_application():
    """Application startup"""
    import os
    from lib import __version__
    from lib.utils.constants import dev_mode
    from lib.utils.logger import Logger

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

    from lib.gui.notifier import notifier
    from lib.utils.translator import translator

    notifier.show_toast(translator.translate("initializing"))

    """Create application files"""
    if not dev_mode:
        from lib.utils.application import application

        application.generate_run()
        application.create_menu_entry()
        application.enable_autostart()

    """Ask for sudo password"""
    from lib.gui.password_dialog import password_dialog
    from lib.utils.configuration import configuration

    if configuration.settings.password is None:
        password_dialog.show()

    """Show tray icon"""
    from lib.gui.tray_icon import tray_icon

    tray_icon.show()

    """Start dbus server"""
    from lib.server.dbus_server import dbus_server

    dbus_server.start()

    """Start autoupdater"""
    from lib.utils.autoupdater import auto_updater

    auto_updater.start()

    logger.rem_tab()
    logger.info("Application ready")


def start_qt_application(application: QApplication):
    """Launch event loop"""
    application.setQuitOnLastWindowClosed(False)
    sys.exit(application.exec_())


if __name__ == "__main__":
    get_application_lock()

    app = create_qt_application()

    initialize_application()

    start_qt_application(app)
