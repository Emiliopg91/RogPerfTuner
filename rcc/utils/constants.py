from datetime import datetime
from pathlib import Path

import os
import shutil
import sys

import toml

DEV_MODE = not hasattr(sys, "frozen") or not sys.frozen  # pylint: disable=no-member

BASE_PATH = Path(os.path.join(os.path.dirname(__file__), "..", "..")).resolve()

AUTOUPDATE_PATH = os.path.join(BASE_PATH, "assets", "autoupdate.json")

ICONS_PATH = os.path.join(BASE_PATH, "assets", "icons")

TRANSLATIONS_PATH = os.path.join(BASE_PATH, "assets", "translations.json")

ORGB_PATH = os.path.join(BASE_PATH, "assets", "OpenRGB", "AppRun")

RCCDC_ASSET_PATH = os.path.join(BASE_PATH, "assets", "RCCDeckyCompanion")

UDEV_PATH = os.path.join(BASE_PATH, "assets", "OpenRGB", "usr", "lib", "udev", "rules.d", "60-openrgb.rules")

PYPROJECT_TOML_PATH = (
    os.path.join(BASE_PATH, "pyproject.toml") if DEV_MODE else os.path.join(BASE_PATH, "assets", "pyproject.toml")
)

with open(PYPROJECT_TOML_PATH, "r") as f:
    data = toml.load(f)
    APP_NAME = data["project"]["name"]
    VERSION = data["project"]["version"]

USER_FOLDER = os.path.expanduser(os.path.join("~", ".config", APP_NAME))
for arg in sys.argv:
    if arg.startswith("--cfg-dir="):
        USER_FOLDER = arg.replace("--cfg-dir=", "")
        print(f"Using user config folder '{USER_FOLDER}'")
        break

OLD_USER_FOLDER = os.path.expanduser(os.path.join("~", APP_NAME))

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

USER_CONFIG_FOLDER = os.path.join(USER_FOLDER, "config")

if not os.path.exists(USER_CONFIG_FOLDER):
    os.makedirs(USER_CONFIG_FOLDER)

ASSET_SCRIPTS_FOLDER = os.path.join(BASE_PATH, "assets", "scripts")

USER_SCRIPTS_FOLDER = os.path.join(USER_FOLDER, "scripts")

if not os.path.exists(USER_SCRIPTS_FOLDER):
    os.makedirs(USER_SCRIPTS_FOLDER)

USER_LOG_FOLDER = os.path.join(USER_FOLDER, "logs")

if not os.path.exists(USER_LOG_FOLDER):
    os.makedirs(USER_LOG_FOLDER)

LOCK_FILE = os.path.join(USER_FOLDER, f"{APP_NAME}.pid")

AUTOSTART_FILE = os.path.expanduser(os.path.join("~", ".config", "autostart", f"{APP_NAME}.desktop"))

APP_DRAW_FILE = os.path.expanduser(os.path.join("~", ".local", "share", "applications", f"{APP_NAME}.desktop"))

CONFIG_FILE = os.path.join(USER_CONFIG_FOLDER, "config.yml")

LOG_OLD_FOLDER = os.path.join(USER_LOG_FOLDER, "old")

LOG_FILE = os.path.join(USER_LOG_FOLDER, f"{datetime.now().strftime("%Y-%m-%d %H:%M:%S")}.log")
