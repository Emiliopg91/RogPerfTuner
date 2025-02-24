import os
import time
from dataclasses import dataclass
from threading import Thread

from framework.logger import Logger
from rcc.communications.client.websocket.steam.steam_client import STEAM_CLIENT
from rcc.models.gpu_brand import GpuBrand
from rcc.models.performance_profile import PerformanceProfile
from rcc.models.platform_profile import PlatformProfile
from rcc.models.settings import GameEntry
from rcc.services.hardware_service import HARDWARE_SERVICE
from rcc.services.profile_service import PROFILE_SERVICE
from rcc.utils.beans import EVENT_BUS
from rcc.utils.configuration import CONFIGURATION
from rcc.utils.constants import RCCDC_ASSET_PATH, USER_PLUGIN_FOLDER
from rcc.utils.events import STEAM_SERVICE_CONNECTED, STEAM_SERVICE_DISCONNECTED, STEAM_SERVICE_GAME_EVENT
from rcc.utils.shell import SHELL


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

    ICD_FILES: dict[GpuBrand, list[str]] = {
        GpuBrand.NVIDIA: [
            "/usr/share/vulkan/icd.d/nvidia_icd.i686.json",
            "/usr/share/vulkan/icd.d/nvidia_icd.x86_64.json",
        ]
    }

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing SteamService")
        self._logger.add_tab()
        self._rccdc_enabled = False
        self.__running_games: dict[int, str] = {}
        self.__gpu: GpuBrand | None = HARDWARE_SERVICE.gpu
        self.__steam_connnected = STEAM_CLIENT.connected

        if self.__gpu is None:
            self._logger.info("No discrete GPU found")
        else:
            if not self.__exists_icd_files(self.__gpu):
                self._logger.warning(f"Missing ICD files for {self.__gpu.value.capitalize()} GPU, discarding")
                self.__gpu = None

        STEAM_CLIENT.on_connected(self.__on_steam_connected)
        STEAM_CLIENT.on_disconnected(self.__on_steam_disconnected)
        STEAM_CLIENT.on_launch_game(self.__launch_game)
        STEAM_CLIENT.on_stop_game(self.__stop_game)

        if STEAM_CLIENT.connected:
            self.__on_steam_connected(True)

        self._logger.rem_tab()

    def __exists_icd_files(self, brand: GpuBrand) -> bool:
        for icd in self.ICD_FILES[brand]:
            if not os.path.exists(icd):
                return False

        return True

    @property
    def gpu(self) -> GpuBrand:
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
        self._logger.info("SteamClient connected")
        self._logger.add_tab()
        if not on_boot:
            self.__set_profile_for_games()
        self._logger.rem_tab()
        self.__steam_connnected = True
        EVENT_BUS.emit(STEAM_SERVICE_CONNECTED)

    def __on_steam_disconnected(self):
        self._logger.info("SteamClient disconnected")
        self.__steam_connnected = False
        PROFILE_SERVICE.restore_profile()
        EVENT_BUS.emit(STEAM_SERVICE_DISCONNECTED)

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
            PROFILE_SERVICE.set_performance_profile(profile, True, name, True)
        else:
            PROFILE_SERVICE.restore_profile()

        EVENT_BUS.emit(STEAM_SERVICE_GAME_EVENT, len(self.__running_games))

    def __get_pids(self, parent_pid):
        pids = []

        output = SHELL.run_command(
            "pstree -p " + str(parent_pid) + " | awk -F'[()]' '{for(i=2;i<=NF;i+=2) print $i}'", False, True, True
        )[1]
        for _ in range(0, 3):
            for line in output.splitlines():
                pid = int(line.strip())
                if pid not in pids:
                    pids.append(pid)
            time.sleep(0.05)

        return pids

    def __launch_game(self, gid: int, name: str, pid: int):
        self._logger.info(f"Launched {name} with PID {pid}")
        if gid not in self.running_games:
            self.__running_games[gid] = name
            self._logger.add_tab()

            pids = self.__get_pids(pid)
            HARDWARE_SERVICE.apply_process_optimizations(pids)
            HARDWARE_SERVICE.set_panel_overdrive(True)
            if CONFIGURATION.games.get(gid) is None:
                CONFIGURATION.games[gid] = GameEntry(name, PerformanceProfile.PERFORMANCE.value)
                CONFIGURATION.save_config()
            self.__set_profile_for_games()
            self._logger.rem_tab()

    def __stop_game(self, gid: int, name: str):
        self._logger.info(f"Stopped {name}")
        if gid in self.running_games:
            del self.running_games[gid]
        self._logger.add_tab()
        HARDWARE_SERVICE.set_panel_overdrive(len(self.running_games) > 0)
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
        SHELL.run_command(f"cp -R {src} {USER_PLUGIN_FOLDER}", False)
        if is_update:
            SHELL.run_command(f"rm -R {dst}", True)
        SHELL.run_command(f"cp -R {os.path.join(USER_PLUGIN_FOLDER, 'RCCDeckyCompanion')} {dst}", True)
        Thread(target=lambda: SHELL.run_command("systemctl restart plugin_loader.service", True)).start()

    def get_games(self) -> dict[str, GameEntry]:
        """Get games and setting"""
        return CONFIGURATION.games

    def set_game_profile(self, game: int, profile: PerformanceProfile = PerformanceProfile.PERFORMANCE):
        """Set profile for game"""
        if CONFIGURATION.games.get(game) is None or CONFIGURATION.games.get(game).profile != profile.value:
            self._logger.info(f"Saving profile {profile.name.lower()} for {CONFIGURATION.games[game].name}")
            CONFIGURATION.games[game].profile = profile.value
            CONFIGURATION.save_config()

    def set_profile_for_running_game(self, profile: PerformanceProfile):
        """If only one game is running store the profile"""
        if len(self.__running_games) == 1:
            game = next(iter(self.__running_games))
            self.set_game_profile(game, profile)
            if PROFILE_SERVICE.performance_profile != profile:
                PROFILE_SERVICE.set_performance_profile(
                    profile, temporal=True, game_name=CONFIGURATION.games[game].name
                )


STEAM_SERVICE = SteamService()
