import os
import subprocess
from threading import Thread

import psutil
from rcc.clients.file.steam_client import steam_client
from rcc.models.settings import Game
from rcc.models.thermal_throttle_profile import ThermalThrottleProfile
from rcc.services.platform_service import platform_service
from rcc.utils import event_bus
from rcc.utils.configuration import configuration
from rcc.utils.constants import rccdc_asset_path, user_plugin_folder
from rcc.utils.cryptography import cryptography
from rcc.utils.logger import Logger


class GamesService:
    """Steam service"""

    def __init__(self):
        self._logger = Logger()
        self._rccdc_enabled = False
        self.__running_games: list[str] = []
        steam_client.on("launch", self.launch_game)
        steam_client.on("stop", self.stop_game)

    @property
    def running_games(self):
        """Get list of running games"""
        return self.__running_games

    @property
    def rccdc_enabled(self):
        """If plugin is enabled"""
        return self._rccdc_enabled

    def install_rccdc(self):
        """Install RCCDeckyCompanion plugin"""
        plugins_folder = os.path.expanduser(os.path.join("~", "homebrew", "plugins"))
        rccdc_path = os.path.join(plugins_folder, "RCCDeckyCompanion")

        if os.path.exists(plugins_folder):
            if not os.path.exists(rccdc_path):
                self._logger.info("Installing plugin for first time")
                self.__copy_plugin(rccdc_asset_path, os.path.join(plugins_folder, "RCCDeckyCompanion"), False)
            else:
                if os.path.getmtime(rccdc_path) < os.path.getmtime(rccdc_asset_path):
                    self._logger.info("Updating plugin")
                    self.__copy_plugin(rccdc_asset_path, os.path.join(plugins_folder, "RCCDeckyCompanion"), True)
                else:
                    self._logger.info("Plugin up to date")
            self._rccdc_enabled = True
        else:
            self._logger.warning("No Decky installation found, skipping")
            self._rccdc_enabled = False

    def __copy_plugin(self, src: str, dst: str, is_update: bool):
        subprocess.run(
            ["cp", "-R", src, user_plugin_folder],
            capture_output=True,
            text=True,
            check=True,
        )
        if is_update:
            subprocess.run(
                ["sudo", "-S", "rm", "-R", dst],
                input=cryptography.decrypt_string(configuration.settings.password) + "\n",
                capture_output=True,
                text=True,
                check=True,
            )
        subprocess.run(
            ["sudo", "-S", "cp", "-R", os.path.join(user_plugin_folder, "RCCDeckyCompanion"), dst],
            input=cryptography.decrypt_string(configuration.settings.password) + "\n",
            capture_output=True,
            text=True,
            check=True,
        )
        with open(os.path.expanduser(os.path.join("~", ".steam", "steam.pid")), "r") as f:
            pid = int(f.read().strip())

        if psutil.pid_exists(pid):
            Thread(
                target=lambda: subprocess.run(
                    ["sudo", "-S", "systemctl", "restart", "plugin_loader.service"],
                    input=cryptography.decrypt_string(configuration.settings.password) + "\n",
                    text=True,
                    check=True,
                )
            ).start()

    def launch_game(self, game: str):
        """Set profile for game"""
        self._logger.info(f"Launched {game}")
        self.__running_games.append(game)
        self._logger.add_tab()
        if game not in configuration.games:
            configuration.games[game] = Game(ThermalThrottleProfile.PERFORMANCE.value)
            configuration.save_config()

        profile = ThermalThrottleProfile(configuration.games[game].profile)
        platform_service.set_thermal_throttle_policy(profile, True, game, True)
        event_bus.event_bus.emit("GamesService.gameEvent", len(self.__running_games))
        self._logger.rem_tab()

    def stop_game(self, game: str):
        """Restore profile after game stop"""
        self._logger.info(f"Stopped {game}")
        self.__running_games.remove(game)
        self._logger.add_tab()
        event_bus.event_bus.emit("GamesService.gameEvent", len(self.__running_games))
        platform_service.restore_profile()
        self._logger.rem_tab()

    def get_games(self) -> dict[str, Game]:
        """Get games asnd setting"""
        return configuration.games

    def set_game_profile(self, game: str, profile: ThermalThrottleProfile):
        """Set profile for game"""
        self._logger.info(f"Setting profile {profile.name.lower()} for {game}")
        configuration.games[game].profile = profile.value
        configuration.save_config()


games_service = GamesService()
