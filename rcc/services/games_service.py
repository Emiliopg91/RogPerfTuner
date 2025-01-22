from dataclasses import dataclass
import os
import subprocess
from threading import Thread

import psutil
from rcc.clients.websocket.steam_client import steam_client
from rcc.models.settings import GameEntry
from rcc.models.platform_profile import PlatformProfile, get_greater
from rcc.services.platform_service import platform_service
from rcc.utils import event_bus
from rcc.utils.configuration import configuration
from rcc.utils.constants import rccdc_asset_path, user_plugin_folder
from rcc.utils.cryptography import cryptography
from rcc.utils.logger import Logger


@dataclass
class RunningGameModel:
    """Running game class"""

    id: int
    name: str


class GamesService:
    """Steam service"""

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing GamesService")
        self._logger.add_tab()
        self._rccdc_enabled = False
        self.__running_games: list[RunningGameModel] = []

        steam_client.on_connected(self.__on_steam_connected)
        steam_client.on_disconnected(self.__on_steam_disconnected)
        steam_client.on_launch_game(self.launch_game)
        steam_client.on_stop_game(self.stop_game)

        if steam_client.connected:
            self.__on_steam_connected()

        self._logger.rem_tab()

    def __on_steam_connected(self):
        self.__running_games = []
        rg = steam_client.get_running_games()
        self._logger.info(f"SteamClient connected. Running {len(rg)} games:")
        self._logger.add_tab()
        for g in rg:
            game = RunningGameModel(g["id"], g["name"])
            self._logger.info(game.name)
            self.__running_games.append(RunningGameModel(game.id, game.name))

        self._logger.rem_tab()
        self.__set_profile_for_games()

    def __on_steam_disconnected(self):
        self._logger.info("SteamClient disconnected")
        if len(self.__running_games) > 0:
            self._logger.info("Restoring platform profile")
            self.__running_games = []
            self._logger.add_tab()
            platform_service.restore_profile()
            self._logger.rem_tab()

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
                    self._logger.info("Updating Decky plugin")
                    self.__copy_plugin(rccdc_asset_path, os.path.join(plugins_folder, "RCCDeckyCompanion"), True)
                else:
                    self._logger.debug("Plugin up to date")
            self._rccdc_enabled = True
        else:
            self._logger.warning("No Decky installation found, skipping plugin installation")
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

    def __set_profile_for_games(
        self,
    ):
        if len(self.__running_games) > 0:
            profile = PlatformProfile.QUIET

            for entry in self.__running_games:
                if entry.id not in configuration.games:
                    configuration.games[entry.id] = GameEntry(entry.name, PlatformProfile.PERFORMANCE.value)
                profile = get_greater(profile, PlatformProfile(configuration.games[entry.id].profile))
            configuration.save_config()

            entry = [
                game.name
                for game in self.__running_games
                if PlatformProfile(configuration.games[game.id].profile) == profile
            ][0]
            platform_service.set_thermal_throttle_policy(profile, True, entry, True)

            event_bus.event_bus.emit("GamesService.gameEvent", len(self.__running_games))

    def launch_game(self, gid: int, name: str):
        """Set profile for game"""
        self._logger.info(f"Launched {name}")
        if not self.__is_game_running(gid):
            self.__running_games.append(RunningGameModel(gid, name))
            self._logger.add_tab()
            if gid not in configuration.games:
                self.set_game_profile(gid, name)

            self.__set_profile_for_games()
            self._logger.rem_tab()

    def stop_game(self, gid: int, name: str):
        """Restore profile after game stop"""
        self._logger.info(f"Stopped {name}")
        if self.__is_game_running(gid):
            self.__running_games = [entry for entry in self.__running_games if entry.id != gid]
            self._logger.add_tab()
            if len(self.__running_games) > 0:
                self.__set_profile_for_games()
            else:
                platform_service.restore_profile()
                event_bus.event_bus.emit("GamesService.gameEvent", len(self.__running_games))
            self._logger.rem_tab()

    def __is_game_running(self, gid: int) -> bool:
        return any(game.id == gid for game in self.__running_games)

    def get_games(self) -> dict[str, GameEntry]:
        """Get games and setting"""
        return configuration.games

    def set_game_profile(self, game: int, profile: PlatformProfile = PlatformProfile.PERFORMANCE):
        """Set profile for game"""
        self._logger.info(f"Saving profile {profile.name.lower()} for {configuration.games[game].name}")
        configuration.games[game].profile = profile.value
        configuration.save_config()


games_service = GamesService()
