import json
import os
import yaml

from lib.models.settings import (
    Config,
    Game,
    Settings,
    Platform,
    PlatformProfiles,
    OpenRgb,
)
from lib.models.boost import Boost
from lib.models.thermal_throttle_profile import ThermalThrottleProfile
from lib.utils.singleton import singleton
from lib.utils.constants import config_file, config_folder


@singleton
class Configuration:
    """Class for accesing to config file data"""

    def __init__(self):
        self._load_config()
        self._config: Config | None

    def _load_config(self) -> None:
        try:
            with open(config_file, "r") as f:
                raw_config = yaml.safe_load(f) or {}

            update = False
            if "boost" not in raw_config["platform"]["profiles"]:
                raw_config["platform"]["profiles"]["boost"] = Boost.AUTO.value
                update = True
            if "games" not in raw_config:
                raw_config["games"] = {}
                update = True
            if "logger" not in raw_config:
                raw_config["logger"] = {}
                update = True

            json_output = json.dumps(raw_config, indent=2)
            self._config = Config.from_json(json_output)  # pylint: disable=E1101

            if update:
                self.save_config()

        except FileNotFoundError:
            if not os.path.exists(config_folder):
                os.makedirs(config_folder, exist_ok=True)

            self._config = Config(
                logger={},
                games={},
                settings=Settings(None),
                platform=Platform(
                    profiles=PlatformProfiles(ThermalThrottleProfile.PERFORMANCE.value, Boost.AUTO.value)
                ),
                open_rgb=OpenRgb(last_effect="Static", effects={}),
            )
            self.save_config()

    def save_config(self) -> None:
        "Persist config to file"
        with open(config_file, "w") as f:
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
    def games(self) -> dict[str, Game]:
        """Getter for games"""
        return self._config.games

    @property
    def logger(self) -> dict[str, str]:
        """Getter for logger"""
        return self._config.logger


configuration = Configuration()
