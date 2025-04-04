#!/bin/env python3

# pylint: disable=import-outside-toplevel, wrong-import-position, ungrouped-imports, wrong-import-order

from rcc.utils.constants import DEV_MODE, USER_UPDATE_FOLDER, LOG_FILE, USER_LOG_FOLDER, LOG_OLD_FOLDER

from framework.logger import Logger

Logger.initialize(LOG_FILE, USER_LOG_FOLDER, LOG_OLD_FOLDER)
logger = Logger("Main")

import asyncio
import os
from qasync import QEventLoop

import setproctitle
from rcc.communications.client.dbus.asus.core.fan_curves_client import FAN_CURVES_CLIENT
from rcc.models.platform_profile import PlatformProfile
from rcc.utils.constants import APP_NAME, VERSION

setproctitle.setproctitle(APP_NAME)

from PyQt5.QtWidgets import QApplication
import sys


def create_qt_application() -> QApplication:
    """Initialize QApplication"""
    q_app = QApplication(sys.argv)
    q_loop = QEventLoop(q_app)
    asyncio.set_event_loop(q_loop)
    return q_app, q_loop


def initialize_application():  # pylint:disable=too-many-locals
    curves = FAN_CURVES_CLIENT.fan_curve_data(PlatformProfile.PERFORMANCE)
    print(curves)
    for curve in curves:
        for i in range(len(curve.points)):  # pylint: disable=consider-using-enumerate
            curve.points[i] = (curve.points[i][0], min(100, round(curve.points[i][1] * 1.15)))
        print(f"{curve}")
        FAN_CURVES_CLIENT.set_fan_curve(PlatformProfile.PERFORMANCE, curve)


if __name__ == "__main__":
    print(os.getpid())

    app, loop = create_qt_application()

    initialize_application()
