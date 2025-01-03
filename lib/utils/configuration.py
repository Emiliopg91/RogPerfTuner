from dataclasses import asdict

import os
import yaml

from lib.models.settings import (
    Config,
    Settings,
    Platform,
    PlatformProfiles,
    OpenRgb,
    Effect,
)
from lib.models.thermal_throttle_profile import ThermalThrottleProfile
from lib.utils.singleton import singleton
from lib.utils.constants import config_file, config_folder
from lib.utils.logger import Logger


@singleton
class Configuration:
    """Class for accesing to config file data"""

    def __init__(self):
        self.logger = Logger()
        self._load_config()

    def _load_config(self) -> None:
        self.logger.debug("Loading settings from file")
        try:
            with open(config_file, "r") as f:
                raw_config = yaml.safe_load(f) or {}
            self.config = Config(
                settings=Settings(**raw_config.get("settings", {})),
                platform=Platform(profiles=PlatformProfiles(**raw_config.get("platform", {}).get("profiles", {}))),
                open_rgb=OpenRgb(
                    last_effect=raw_config.get("open_rgb", {}).get("last_effect", "Static"),
                    effects={
                        name: Effect(**effect)
                        for name, effect in raw_config.get("open_rgb", {}).get("effects", {}).items()
                    },
                ),
            )
        except FileNotFoundError:
            if not os.path.exists(config_folder):
                os.makedirs(config_folder, exist_ok=True)

            self.config = Config(
                settings=Settings(None),
                platform=Platform(profiles=PlatformProfiles(ThermalThrottleProfile.PERFORMANCE.value)),
                open_rgb=OpenRgb(last_effect="Static", effects={}),
            )
            self.save_config()

    def save_config(self) -> None:
        "Persist config to file"
        self.logger.debug("Persisting settings to file")
        with open(config_file, "w") as f:
            yaml.dump(asdict(self.config), f, default_flow_style=False)

    @property
    def settings(self) -> Settings:
        """Getter for settings"""
        return self.config.settings

    @property
    def open_rgb(self) -> OpenRgb:
        """Getter for openrgb"""
        return self.config.open_rgb

    @property
    def platform(self) -> Platform:
        """Getter for platform"""
        return self.config.platform


configuration = Configuration()
