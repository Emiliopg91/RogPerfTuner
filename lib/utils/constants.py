from pathlib import Path

import os
import sys

dev_mode = not hasattr(sys, "frozen") or not sys.frozen

base_path = Path(os.path.join(os.path.dirname(__file__), "..", "..")).resolve()

autoupdate_path = os.path.join(base_path, "assets", "autoupdate.json")

icons_path = os.path.join(base_path, "assets", "icons")

translations_path = os.path.join(base_path, "assets", "translations.json")

orgb_path = os.path.join(base_path, "assets", "OpenRGB.AppImage")

user_folder = os.path.expanduser(os.path.join("~", "RogControlCenter"))

if not os.path.exists(user_folder):
    os.makedirs(user_folder)

lock_file = os.path.join(user_folder, ".lock")

config_folder = os.path.join(user_folder, "config")

scripts_folder = os.path.join(user_folder, "scripts")

config_file = os.path.join(config_folder, "config.yml")

log_folder = os.path.join(user_folder, "logs")

log_file = os.path.join(log_folder, "application.log")
