import os
import sys

from PyQt5.QtCore import QObject, pyqtSlot, Q_CLASSINFO
from PyQt5.QtDBus import QDBusConnection, QDBusAbstractAdaptor

from rcc.services.steam_service import STEAM_SERVICE
from rcc.services.rgb_service import RGB_SERVICE
from rcc.services.profile_service import PROFILE_SERVICE
from rcc.utils.constants import USER_SCRIPTS_FOLDER
from framework.logger import Logger
from framework.singleton import singleton


SERVICE_NAME = "es.emiliopg91.RogControlCenter"
OBJECT_PATH = "/es/emiliopg91/RogControlCenter"
INTERFACE_NAME = "es.emiliopg91.RogControlCenter"


@singleton
class HelloService(QObject):
    """Dbus service"""

    def next_profile(self):
        """Activate next profile"""
        if len(STEAM_SERVICE.running_games) == 0:
            next_t = PROFILE_SERVICE.performance_profile.next_performance_profile
            PROFILE_SERVICE.set_performance_profile(next_t)
            return next_t.name

        if len(STEAM_SERVICE.running_games) == 1:
            next_t = PROFILE_SERVICE.performance_profile.next_performance_profile
            STEAM_SERVICE.set_profile_for_running_game(next_t)
            return next_t.name

        return "Not available on multi-game session"

    def next_effect(self):
        """Activate next effect"""
        next_t = RGB_SERVICE.get_next_effect()
        RGB_SERVICE.apply_effect(next_t)
        return next_t

    def increase_brightness(self):
        """Increase brightness"""
        next_t = RGB_SERVICE.brightness.get_next_brightness()
        RGB_SERVICE.apply_brightness(next_t)
        return next_t.name

    def decrease_brightness(self):
        """Decrease brightness"""
        next_t = RGB_SERVICE.brightness.get_previous_brightness()
        RGB_SERVICE.apply_brightness(next_t)
        return next_t.name


@singleton
class HelloServiceAdaptor(QDBusAbstractAdaptor):
    """Adaptor for dbus service"""

    Q_CLASSINFO("D-Bus Interface", INTERFACE_NAME)

    def __init__(self, service: HelloService):
        super().__init__(service)
        self.service = service

    @pyqtSlot(result=str, name="nextProfile")
    def next_profile(self):
        """Activate next profile"""
        return self.service.next_profile()

    @pyqtSlot(result=str, name="nextEffect")
    def next_effect(self):
        """Activate next effect"""
        return self.service.next_effect()

    @pyqtSlot(result=str, name="increaseBrightness")
    def increase_brightness(self):
        """Increase brightness"""
        return self.service.increase_brightness()

    @pyqtSlot(result=str, name="decreaseBrightness")
    def decrease_brightness(self):
        """Decrease brightness"""
        return self.service.decrease_brightness()


@singleton
class DBusServer:
    """Dbus server to expose functionality"""

    def __init__(self):
        self._logger = Logger()
        self._file_actions: dict[str, str] = {
            "decBrightness.sh": "decreaseBrightness",
            "incBrightness.sh": "increaseBrightness",
            "nextAnimation.sh": "nextEffect",
            "nextProfile.sh": "nextProfile",
        }

        os.makedirs(USER_SCRIPTS_FOLDER, exist_ok=True)

        for file, action in self._file_actions.items():
            file_path = os.path.join(USER_SCRIPTS_FOLDER, file)
            script_content = f"""#!/bin/bash
gdbus call --session --dest {SERVICE_NAME} --object-path {OBJECT_PATH} --method {INTERFACE_NAME}.{action}
            """

            with open(file_path, "w") as f:
                f.write(script_content)

            os.chmod(file_path, 0o755)

    def start(self):
        """Start dbus server"""

        self._logger.info("Initializing DBusServer")
        session_bus = QDBusConnection.sessionBus()

        if not session_bus.registerService(SERVICE_NAME):
            self._logger.error(f"Error: Couldn't register service {SERVICE_NAME}")
            sys.exit(1)

        service = HelloService()
        adaptor = HelloServiceAdaptor(service)

        if not session_bus.registerObject(OBJECT_PATH, service):
            self._logger.error(f"Error: Couldn't register object {OBJECT_PATH}")
            sys.exit(1)

        session_bus.registerObject(OBJECT_PATH, adaptor)


DBUS_SERVER = DBusServer()
