import json
import os
import yaml

from rcc.communications.client.tcp.openrgb.effects.static import STATIC_EFFECT
from rcc.models.rgb_brightness import RgbBrightness
from rcc.models.settings import (
    Config,
    GameEntry,
    Settings,
    Platform,
    PlatformProfiles,
    OpenRgb,
)
from rcc.models.performance_profile import PerformanceProfile
from rcc.utils.constants import CONFIG_FILE, USER_CONFIG_FOLDER
from framework.singleton import singleton
from framework.logger import Logger


@singleton
class Configuration:
    """Class for accesing to config file data"""

    def __init__(self):
        self._load_config()
        self._config: Config | None

    def _load_config(self) -> None:
        if os.path.exists(CONFIG_FILE):
            with open(CONFIG_FILE, "r") as f:
                raw_config = yaml.safe_load(f) or {}

            json_output = json.dumps(raw_config, indent=2)
            self._config = Config.from_json(json_output)
            Logger.set_config_map(self._config.logger)
        else:
            if not os.path.exists(USER_CONFIG_FOLDER):
                os.makedirs(USER_CONFIG_FOLDER, exist_ok=True)

            self._config = Config(
                logger={},
                games={},
                settings=Settings(None),
                platform=Platform(profiles=PlatformProfiles(PerformanceProfile.PERFORMANCE.value)),
                open_rgb=OpenRgb(last_effect=STATIC_EFFECT.name, brightness=RgbBrightness.MAX.value, effects={}),
            )
            self.save_config()

    def save_config(self) -> None:
        "Persist config to file"
        with open(CONFIG_FILE, "w") as f:
            yaml.dump(self._config.to_dict(), f, default_flow_style=False)

    @property
    def settings(self) -> Settings:
        """Getter for settings"""
        return self._config.settings

    @property
    def open_rgb(self) -> OpenRgb:
        """Getter for openrgb"""
        return self._config.open_rgb

    @property
    def platform(self) -> Platform:
        """Getter for platform"""
        return self._config.platform

    @property
    def games(self) -> dict[int, GameEntry]:
        """Getter for games"""
        return self._config.games

    @property
    def logger(self) -> dict[str, str]:
        """Getter for logger"""
        return self._config.logger


CONFIGURATION = Configuration()
