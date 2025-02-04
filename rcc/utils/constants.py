from datetime import datetime
from pathlib import Path

import os
import shutil
import sys

from rcc import __app_name__

# pylint: disable=E1101
DEV_MODE = not hasattr(sys, "frozen") or not sys.frozen

USER_FOLDER = os.path.expanduser(os.path.join("~", ".config", __app_name__))

OLD_USER_FOLDER = os.path.expanduser(os.path.join("~", __app_name__))

if os.path.exists(OLD_USER_FOLDER):
    shutil.move(OLD_USER_FOLDER, USER_FOLDER)

if not os.path.exists(USER_FOLDER):
    os.makedirs(USER_FOLDER)

USER_ICON_FOLDER = os.path.join(USER_FOLDER, "icons")

if not os.path.exists(USER_ICON_FOLDER):
    os.makedirs(USER_ICON_FOLDER)

USER_PLUGIN_FOLDER = os.path.join(USER_FOLDER, "plugin")

if not os.path.exists(USER_PLUGIN_FOLDER):
    os.makedirs(USER_PLUGIN_FOLDER)

USER_UPDATE_FOLDER = os.path.join(USER_FOLDER, "update")

if not os.path.exists(USER_UPDATE_FOLDER):
    os.makedirs(USER_UPDATE_FOLDER)

USER_EFFECTS_FOLDER = os.path.join(USER_FOLDER, "effects")

if not os.path.exists(USER_EFFECTS_FOLDER):
    os.makedirs(USER_EFFECTS_FOLDER)

USER_BIN_FOLDER = os.path.join(USER_FOLDER, "bin")

if not os.path.exists(USER_BIN_FOLDER):
    os.makedirs(USER_BIN_FOLDER)

AUTOSTART_FILE = os.path.expanduser(os.path.join("~", ".config", "autostart", f"{__app_name__}.desktop"))

APP_DRAW_FILE = os.path.expanduser(os.path.join("~", ".local", "share", "applications", f"{__app_name__}.desktop"))

BASE_PATH = Path(os.path.join(os.path.dirname(__file__), "..", "..")).resolve()

AUTOUPDATE_PATH = os.path.join(BASE_PATH, "assets", "autoupdate.json")

ICONS_PATH = os.path.join(BASE_PATH, "assets", "icons")

TRANSLATIONS_PATH = os.path.join(BASE_PATH, "assets", "translations.json")

ORGB_PATH = os.path.join(BASE_PATH, "assets", "OpenRGB.AppImage")

RCCDC_ASSET_PATH = os.path.join(BASE_PATH, "assets", "RCCDeckyCompanion")

UDEV_PATH = os.path.join(BASE_PATH, "assets", "60-openrgb.rules")

LOCK_FILE = os.path.join(USER_FOLDER, f"{__app_name__}.pid")

CONFIG_FOLDER = os.path.join(USER_FOLDER, "config")

SCRIPTS_FOLDER = os.path.join(USER_FOLDER, "scripts")

CONFIG_FILE = os.path.join(CONFIG_FOLDER, "config.yml")

LOG_FOLDER = os.path.join(USER_FOLDER, "logs")

LOG_OLD_FOLDER = os.path.join(LOG_FOLDER, "old")

LOG_FILE = os.path.join(LOG_FOLDER, f"{datetime.now().strftime("%Y-%m-%d %H:%M:%S")}.log")
