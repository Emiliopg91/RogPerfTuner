from ..models.thermal_throttle_profile import getNextThermalThrottleProfile
from ..models.rgb_brightness import getNextBrightness, getPreviousBrightness
from ..services.openrgb_service import open_rgb_service
from ..services.platform_service import platform_service
from ..utils.constants import scripts_folder
from ..utils.logger import Logger
from ..utils.singleton import singleton

from PyQt5.QtCore import QObject, pyqtSlot, Q_CLASSINFO
from PyQt5.QtDBus import QDBusConnection, QDBusAbstractAdaptor

import os
import sys


SERVICE_NAME = "es.emiliopg91.RogControlCenter"
OBJECT_PATH = "/es/emiliopg91/RogControlCenter"
INTERFACE_NAME = "es.emiliopg91.RogControlCenter"

@singleton
class HelloService(QObject):
    def __init__(self):
        super().__init__()

    def nextProfile(self):
        current = platform_service.thermal_throttle_profile
        next = getNextThermalThrottleProfile(current)
        platform_service.set_thermal_throttle_policy(next)
        return next.name

    def nextEffect(self):
        next = open_rgb_service.get_next_effect()
        open_rgb_service.apply_effect(next)
        return next

    def increaseBrightness(self):
        current = open_rgb_service.brightness
        next = getNextBrightness(current)
        open_rgb_service.apply_brightness(next)
        return next.name

    def decreaseBrightness(self):
        current = open_rgb_service.brightness
        next = getPreviousBrightness(current)
        open_rgb_service.apply_brightness(next)
        return next.name

@singleton
class HelloServiceAdaptor(QDBusAbstractAdaptor):
    Q_CLASSINFO("D-Bus Interface", INTERFACE_NAME)
    def __init__(self, service:HelloService):
        super().__init__(service)
        self.service = service

    @pyqtSlot(result=str)
    def nextProfile(self):
        return self.service.nextProfile()

    @pyqtSlot(result=str)
    def nextEffect(self):
        return self.service.nextEffect()

    @pyqtSlot(result=str)
    def increaseBrightness(self):
        return self.service.increaseBrightness()

    @pyqtSlot(result=str)
    def decreaseBrightness(self):
        return self.service.decreaseBrightness()

@singleton
class DBusServer:    
    def __init__(self):
        self.logger = Logger(self.__class__.__name__)
        self.file_actions: dict[str, str] = {
                'decBrightness.sh': "decreaseBrightness",
                'incBrightness.sh': "increaseBrightness",
                'nextAnimation.sh': "nextEffect",
                'nextProfile.sh': "nextProfile"
            }

        os.makedirs(scripts_folder, exist_ok=True)

        for file, action in self.file_actions.items():
            file_path = os.path.join(scripts_folder, file)
            script_content = f"""#!/bin/bash
                gdbus call --session --dest {SERVICE_NAME} --object-path {OBJECT_PATH} --method {INTERFACE_NAME}.{action}
            """
            
            with open(file_path, 'w') as f:
                f.write(script_content)
                
            os.chmod(file_path, 0o755)

    def start(self):
        session_bus = QDBusConnection.sessionBus()

        if not session_bus.registerService(SERVICE_NAME):
            self.logger.error(f"Error: Couldn't register service {SERVICE_NAME}")
            sys.exit(1)

        service = HelloService()
        adaptor = HelloServiceAdaptor(service)

        if not session_bus.registerObject(OBJECT_PATH, service):
            self.logger.error(f"Error: Couldn't register object {OBJECT_PATH}")
            sys.exit(1)

        session_bus.registerObject(OBJECT_PATH, adaptor)

        self.logger.info(f"D-Bus service started")

dbus_server = DBusServer()