from datetime import datetime
from pathlib import Path

import os
import shutil
import sys

from lib import __app_name__

# pylint: disable=E1101
dev_mode = not hasattr(sys, "frozen") or not sys.frozen

user_folder = os.path.expanduser(os.path.join("~", ".config", __app_name__))

old_user_folder = os.path.expanduser(os.path.join("~", __app_name__))

if os.path.exists(old_user_folder):
    shutil.move(old_user_folder, user_folder)

if not os.path.exists(user_folder):
    os.makedirs(user_folder)

user_icon_folder = os.path.join(user_folder, "icons")

if not os.path.exists(user_icon_folder):
    os.makedirs(user_icon_folder)

user_plugin_folder = os.path.join(user_folder, "plugin")

if not os.path.exists(user_plugin_folder):
    os.makedirs(user_plugin_folder)

user_update_folder = os.path.join(user_folder, "update")

if not os.path.exists(user_update_folder):
    os.makedirs(user_update_folder)

user_bin_folder = os.path.join(user_folder, "bin")

if not os.path.exists(user_bin_folder):
    os.makedirs(user_bin_folder)

autostart_file = os.path.expanduser(os.path.join("~", ".config", "autostart", f"{__app_name__}.desktop"))

app_draw_file = os.path.expanduser(os.path.join("~", ".local", "share", "applications", f"{__app_name__}.desktop"))

base_path = Path(os.path.join(os.path.dirname(__file__), "..", "..")).resolve()

autoupdate_path = os.path.join(base_path, "assets", "autoupdate.json")

icons_path = os.path.join(base_path, "assets", "icons")

translations_path = os.path.join(base_path, "assets", "translations.json")

orgb_path = os.path.join(base_path, "assets", "OpenRGB.AppImage")

rccdc_asset_path = os.path.join(base_path, "assets", "RCCDeckyCompanion")

udev_path = os.path.join(base_path, "assets", "60-openrgb.rules")

lock_file = os.path.join(user_folder, ".lock")

config_folder = os.path.join(user_folder, "config")

scripts_folder = os.path.join(user_folder, "scripts")

config_file = os.path.join(config_folder, "config.yml")

log_folder = os.path.join(user_folder, "logs")

log_old_folder = os.path.join(log_folder, "old")

log_file = os.path.join(log_folder, f"{datetime.now().strftime("%Y-%m-%d %H:%M:%S")}.log")
