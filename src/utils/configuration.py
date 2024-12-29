from ..models.rgb_brightness import RgbBrightness
from ..models.settings import *
from ..models.thermal_throttle_profile import ThermalThrottleProfile
from ..utils.singleton import singleton
from .constants import config_file, config_folder
from .logger import Logger

from dataclasses import asdict

import os
import yaml


@singleton
class Configuration:
    def __init__(self):
        self.logger = Logger("Configuration")
        self._load_config()

    def _load_config(self):
        self.logger.debug("Loading settings from file")
        try:
            with open(config_file, "r") as f:
                raw_config = yaml.safe_load(f) or {}
            self.config = Config(
                settings=Settings(**raw_config.get("settings", {})),
                platform=Platform(
                    profiles=PlatformProfiles(
                        **raw_config.get("platform", {}).get("profiles", {})
                    )
                ),
                openRgb=OpenRgb(
                    lastEffect=raw_config.get("openRgb", {}).get(
                        "lastEffect", "Static"
                    ),
                    effects={
                        name: Effect(**effect)
                        for name, effect in raw_config.get("openRgb", {})
                        .get("effects", {})
                        .items()
                    },
                ),
            )
        except FileNotFoundError:
            if not os.path.exists(config_folder):
                os.makedirs(config_folder, exist_ok=True)

            self.config = Config(
                settings=Settings(None),
                platform=Platform(
                    profiles=PlatformProfiles(ThermalThrottleProfile.PERFORMANCE.value)
                ),
                openRgb=OpenRgb(lastEffect="Static", effects={}),
            )
            self.save_config()

    def save_config(self):
        self.logger.debug("Persisting settings to file")
        with open(config_file, "w") as f:
            yaml.dump(asdict(self.config), f, default_flow_style=False)

    @property
    def settings(self) -> Settings:
        return self.config.settings

    @property
    def openRgb(self) -> OpenRgb:
        return self.config.openRgb

    @property
    def platform(self) -> Platform:
        return self.config.platform


configuration = Configuration()
