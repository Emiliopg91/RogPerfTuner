from dataclasses import dataclass
import os
import subprocess
from threading import Thread

from rcc.communications.client.websocket.steam.steam_client import STEAM_CLIENT
from rcc.models.performance_profile import PerformanceProfile
from rcc.models.settings import GameEntry
from rcc.models.platform_profile import PlatformProfile
from rcc.services.performance_service import PERFORMANCE_SERVICE
from rcc.utils.configuration import CONFIGURATION
from rcc.utils.constants import RCCDC_ASSET_PATH, USER_PLUGIN_FOLDER
from rcc.utils.beans import CRYPTOGRAPHY
from rcc.utils.beans import EVENT_BUS
from rcc.utils.events import STEAM_SERVICE_CONNECTED, STEAM_SERVICE_DISCONNECTED, STEAM_SERVICE_GAME_EVENT
from framework.logger import Logger


@dataclass
class RunningGameModel:
    """Running game class"""

    id: int
    name: str


class SteamService:
    """Steam service"""

    DECKY_SERVICE_PATH = os.path.expanduser(os.path.join("~", "homebrew", "services", "PluginLoader"))
    PLUGINS_FOLDER = os.path.expanduser(os.path.join("~", "homebrew", "plugins"))
    RCCDC_PATH = os.path.join(PLUGINS_FOLDER, "RCCDeckyCompanion")

    ICD_FILES: dict[str, list[str]] = {
        "nvidia": ["/usr/share/vulkan/icd.d/nvidia_icd.i686.json", "/usr/share/vulkan/icd.d/nvidia_icd.x86_64.json"]
    }

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing GamesService")
        self._logger.add_tab()
        self._rccdc_enabled = False
        self.__running_games: dict[int, str] = {}
        self.__gpu: str | None = None
        self.__steam_connnected = STEAM_CLIENT.connected

        lspci_result = subprocess.run(["lspci"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)
        output = lspci_result.stdout
        lines = [line for line in output.splitlines() if "VGA" in line or "3D" in line]
        for _, line in enumerate(lines):
            if line.split(": ")[1].strip().lower().startswith("nvidia"):
                self.__gpu = "nvidia"
                break

        if self.__gpu is None:
            self._logger.info("No discrete GPU found")
        else:
            self._logger.info(f"Found discrete {self.__gpu.capitalize()} GPU")
            if not self.__exists_icd_files("nvidia"):
                self._logger.warning(f"Missing ICD files for {self.__gpu.capitalize()} GPU, discarding")
                self.__gpu = None

        STEAM_CLIENT.on_connected(self.__on_steam_connected)
        STEAM_CLIENT.on_disconnected(self.__on_steam_disconnected)
        STEAM_CLIENT.on_launch_game(self.__launch_game)
        STEAM_CLIENT.on_stop_game(self.__stop_game)

        if STEAM_CLIENT.connected:
            self.__on_steam_connected(True)

        self._logger.rem_tab()

    def __exists_icd_files(self, brand: str) -> bool:
        for icd in self.ICD_FILES[brand]:
            if not os.path.exists(icd):
                return False

        return True

    @property
    def gpu(self):
        """GPU brand"""
        return self.__gpu

    @property
    def icd_files(self) -> list[str] | None:
        """ICD files for GPU"""
        if self.__gpu is not None:
            return self.ICD_FILES[self.__gpu]
        return None

    def __on_steam_connected(self, on_boot=False):
        self.__running_games = {}
        rg = STEAM_CLIENT.get_running_games()
        self._logger.info(f"SteamClient connected. Running {len(rg)} games")
        self._logger.add_tab()
        if len(rg) > 0:
            for g in rg:
                game = RunningGameModel(g["id"], g["name"])
                self._logger.info(game.name)
                self.__running_games[game.id] = game.name
            self.__set_profile_for_games()
        elif not on_boot:
            self.__set_profile_for_games()
        self._logger.rem_tab()
        self.__steam_connnected = True
        EVENT_BUS.emit(STEAM_SERVICE_CONNECTED)

    def __on_steam_disconnected(self):
        self._logger.info("SteamClient disconnected")
        self.__steam_connnected = False
        EVENT_BUS.emit(STEAM_SERVICE_DISCONNECTED)
        if len(self.__running_games) > 0:
            self._logger.info("Restoring platform profile")
            self.__running_games = {}
            self._logger.add_tab()
            PERFORMANCE_SERVICE.restore_profile()
            self._logger.rem_tab()

    @property
    def steam_connected(self):
        """Flag for steam connection"""
        return self.__steam_connnected

    def __set_profile_for_games(self):
        if len(self.__running_games) > 0:
            profile = PerformanceProfile.QUIET

            for gid in self.__running_games:
                profile = profile.get_greater(PlatformProfile(CONFIGURATION.games[gid].profile))

            name = [
                CONFIGURATION.games[gid].name
                for gid in self.__running_games
                if PlatformProfile(CONFIGURATION.games[gid].profile) == profile
            ][0]
            PERFORMANCE_SERVICE.set_performance_profile(profile, True, name, True)
        else:
            PERFORMANCE_SERVICE.restore_profile()

        EVENT_BUS.emit(STEAM_SERVICE_GAME_EVENT, len(self.__running_games))

    def __launch_game(self, gid: int, name: str):
        self._logger.info(f"Launched {name}")
        if gid not in self.running_games:
            self.__running_games[gid] = name
            self._logger.add_tab()
            if gid not in CONFIGURATION.games:
                CONFIGURATION.games[gid] = GameEntry(name, PlatformProfile.PERFORMANCE.value)
                CONFIGURATION.save_config()

            self.__set_profile_for_games()
            self._logger.rem_tab()

    def __stop_game(self, gid: int, name: str):
        self._logger.info(f"Stopped {name}")
        if gid in self.running_games:
            del self.running_games[gid]
            self._logger.add_tab()
            self.__set_profile_for_games()
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

        if os.path.exists(self.DECKY_SERVICE_PATH):
            if not os.path.exists(self.RCCDC_PATH):
                self._logger.info("Installing plugin for first time")
                self.__copy_plugin(RCCDC_ASSET_PATH, os.path.join(self.PLUGINS_FOLDER, "RCCDeckyCompanion"), False)
            else:
                if os.path.getmtime(self.RCCDC_PATH) < os.path.getmtime(RCCDC_ASSET_PATH):
                    self._logger.info("Updating Decky plugin")
                    self.__copy_plugin(RCCDC_ASSET_PATH, os.path.join(self.PLUGINS_FOLDER, "RCCDeckyCompanion"), True)
                else:
                    self._logger.debug("Plugin up to date")
            self._rccdc_enabled = True
        else:
            self._logger.warning("No Decky installation found, skipping plugin installation")
            self._rccdc_enabled = False

    def __copy_plugin(self, src: str, dst: str, is_update: bool):
        subprocess.run(
            f"cp -R {src} {USER_PLUGIN_FOLDER}",
            capture_output=True,
            text=True,
            shell=True,
            check=True,
        )
        if is_update:
            subprocess.run(
                f"sudo -S rm -R {dst}",
                input=CRYPTOGRAPHY.decrypt_string(CONFIGURATION.settings.password) + "\n",
                capture_output=True,
                text=True,
                shell=True,
                check=True,
            )
        subprocess.run(
            f"sudo -S cp -R {os.path.join(USER_PLUGIN_FOLDER, 'RCCDeckyCompanion')} dst",
            input=CRYPTOGRAPHY.decrypt_string(CONFIGURATION.settings.password) + "\n",
            capture_output=True,
            text=True,
            shell=True,
            check=True,
        )
        Thread(
            target=lambda: subprocess.run(
                "sudo -S systemctl restart plugin_loader.service",
                input=CRYPTOGRAPHY.decrypt_string(CONFIGURATION.settings.password) + "\n",
                text=True,
                shell=True,
                check=True,
            )
        ).start()

    def get_games(self) -> dict[str, GameEntry]:
        """Get games and setting"""
        return CONFIGURATION.games

    def set_game_profile(self, game: int, profile: PerformanceProfile = PerformanceProfile.PERFORMANCE):
        """Set profile for game"""
        self._logger.info(f"Saving profile {profile.name.lower()} for {CONFIGURATION.games[game].name}")
        CONFIGURATION.games[game].profile = profile.value
        CONFIGURATION.save_config()

    def set_profile_for_running_game(self, profile: PerformanceProfile):
        """If only one game is running store the profile"""
        if len(self.__running_games) == 1:
            game = next(iter(self.__running_games))
            self.set_game_profile(game, profile)
            if PERFORMANCE_SERVICE.performance_profile != profile:
                PERFORMANCE_SERVICE.set_performance_profile(
                    profile, temporal=True, game_name=CONFIGURATION.games[game].name
                )


STEAM_SERVICE = SteamService()
