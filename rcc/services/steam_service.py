import os
from dataclasses import dataclass
from signal import Signals
from threading import Thread
import time

from psutil import NoSuchProcess, Process

from framework.logger import Logger
from rcc.communications.client.cmd.linux.mangohud_client import MANGO_HUD_CLIENT
from rcc.communications.client.cmd.linux.systemctl_client import SYSTEM_CTL_CLIENT
from rcc.communications.client.tcp.openrgb.effects.gaming import GAMING_EFFECT
from rcc.communications.client.websocket.steam.steam_client import STEAM_CLIENT
from rcc.models.gpu_brand import GpuBrand
from rcc.models.mangohud_level import MangoHudLevel
from rcc.models.settings import GameEntry
from rcc.models.wine_sync_option import WineSyncOption
from rcc.models.performance_profile import PerformanceProfile
from rcc.services.hardware_service import HARDWARE_SERVICE
from rcc.services.profile_service import PROFILE_SERVICE
from rcc.services.rgb_service import RGB_SERVICE
from rcc.utils.beans import EVENT_BUS
from rcc.utils.configuration import CONFIGURATION
from rcc.utils.constants import DEV_MODE, RCCDC_ASSET_PATH, USER_PLUGIN_FOLDER, USER_SCRIPTS_FOLDER
from rcc.utils.events import STEAM_SERVICE_CONNECTED, STEAM_SERVICE_DISCONNECTED, STEAM_SERVICE_GAME_EVENT
from rcc.utils.shell import SHELL


@dataclass
class RunningGameModel:
    """Running game class"""

    id: int
    name: str


class SteamService:
    """Steam service"""

    WRAPPER_PATH = os.path.join(USER_SCRIPTS_FOLDER, "wrapper")

    DECKY_SERVICE_PATH = os.path.expanduser(os.path.join("~", "homebrew", "services", "PluginLoader"))
    PLUGINS_FOLDER = os.path.expanduser(os.path.join("~", "homebrew", "plugins"))
    RCCDC_PATH = os.path.join(PLUGINS_FOLDER, "RCCDeckyCompanion")

    def __init__(self):
        self._logger = Logger()
        self._logger.info("Initializing SteamService")
        self._logger.add_tab()
        self._rccdc_enabled = False
        self.__running_games: dict[int, str] = {}
        self.__steam_connnected = STEAM_CLIENT.connected

        STEAM_CLIENT.on_connected(self.__on_steam_connected)
        STEAM_CLIENT.on_disconnected(self.__on_steam_disconnected)
        STEAM_CLIENT.on_launch_game(self.__launch_game)
        STEAM_CLIENT.on_stop_game(self.__stop_game)

        if STEAM_CLIENT.connected:
            self.__on_steam_connected(True)

        self._logger.rem_tab()

    def __on_steam_connected(self, on_boot=False):
        self.__running_games = {}
        self._logger.info("SteamClient connected")
        self._logger.add_tab()
        if not on_boot:
            self.__set_profile_for_games(True)
        self._logger.rem_tab()
        self.__steam_connnected = True
        EVENT_BUS.emit(STEAM_SERVICE_CONNECTED)

    def __on_steam_disconnected(self):
        self._logger.info("SteamClient disconnected")
        self.__steam_connnected = False
        if self.running_games:
            PROFILE_SERVICE.restore_profile()
        EVENT_BUS.emit(STEAM_SERVICE_DISCONNECTED)

    @property
    def steam_connected(self):
        """Flag for steam connection"""
        return self.__steam_connnected

    @property
    def metrics_enabled(self):
        """Flag metrics availability"""
        return MANGO_HUD_CLIENT.available

    def get_games(self) -> dict[int, GameEntry]:
        """Get games and setting"""
        return CONFIGURATION.games

    def __set_profile_for_games(self, on_connect=False):
        if len(self.__running_games) > 0:
            PROFILE_SERVICE.set_performance_profile(PerformanceProfile.PERFORMANCE, True, True)
        elif not on_connect:
            PROFILE_SERVICE.restore_profile()

        EVENT_BUS.emit(STEAM_SERVICE_GAME_EVENT, len(self.__running_games))

    def __launch_game(self, gid: int, name: str, pid: int):
        self._logger.info(f"Launched {name} with PID {pid}")
        self._logger.add_tab()

        if CONFIGURATION.games.get(gid) is None:
            self._logger.info("Game not configured, stopping...")
            process = Process(pid)
            env = process.environ()
            processes = process.children(recursive=True)
            processes.append(process)

            for child in reversed(processes):
                try:
                    child.send_signal(Signals.SIGKILL)
                except NoSuchProcess:
                    pass

            Thread(target=lambda: self.__first_game_run(gid, name, env)).start()
        elif gid not in self.running_games:
            self.__running_games[gid] = name
            HARDWARE_SERVICE.set_panel_overdrive(True)
            self.__set_profile_for_games()
            RGB_SERVICE.apply_effect(GAMING_EFFECT.name, True)

        self._logger.rem_tab()

    def __stop_game(self, gid: int, name: str):
        self._logger.info(f"Stopped {name}")
        pref_running_games = len(self.running_games)
        if gid in self.running_games:
            del self.running_games[gid]
        if pref_running_games != len(self.running_games):
            if len(self.running_games.keys()) == 0:
                RGB_SERVICE.restore_effect()
            self._logger.add_tab()
            HARDWARE_SERVICE.set_panel_overdrive(len(self.running_games) > 0)
            self.__set_profile_for_games()
            self._logger.rem_tab()

    def __first_game_run(self, gid: int, name: str, env: dict[str, str]):
        self._logger.info("Retrieving game details...")

        gpu = None
        if len(HARDWARE_SERVICE.gpus) > 0:
            for gpu_brand in GpuBrand:
                if gpu_brand in HARDWARE_SERVICE.gpus:
                    gpu = gpu_brand.value
                    break

        entry = GameEntry(name, gpu=gpu, proton="STEAM_COMPAT_PROTON" in env)
        details = STEAM_CLIENT.get_apps_details(gid)[0]
        launch_opts = details.launch_opts

        if "%command" in launch_opts:
            entry.env = launch_opts[0 : launch_opts.index("%command%")].strip()
            launch_opts = launch_opts[launch_opts.index("%command%") + 9 :].strip()

        if len(launch_opts) > 0:
            entry.args = launch_opts

        CONFIGURATION.games[gid] = entry
        CONFIGURATION.save_config()

        self._logger.info("Updating launch options...")
        STEAM_CLIENT.set_launch_options(gid, f"{self.WRAPPER_PATH} %command%")

        overlay_id = env["SteamOverlayGameId"]
        self._logger.info(f"Relaunching with Overlay GameId {overlay_id}...")
        time.sleep(1)
        SHELL.run_command(f"{env["STEAMSCRIPT"]} steam://rungameid/{overlay_id}")
        self._logger.info("Relauched")

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
        Thread(target=lambda: SYSTEM_CTL_CLIENT.restart_service("plugin_loader")).start()

    def get_metrics_level(self, app_id) -> MangoHudLevel:
        """Get level for game"""
        game = CONFIGURATION.games.get(app_id)
        return (
            MangoHudLevel(game.metrics_level) if game and game.metrics_level is not None else MangoHudLevel.NO_DISPLAY
        )

    def set_metrics_level(self, metric_level: MangoHudLevel, app_id) -> MangoHudLevel:
        """Set level for game launch option"""
        game = CONFIGURATION.games.get(app_id)
        game.metrics_level = metric_level.value if metric_level is not None else MangoHudLevel.NO_DISPLAY.value
        CONFIGURATION.save_config()

    def get_wine_sync(self, app_id) -> WineSyncOption:
        """Get level for game"""
        game = CONFIGURATION.games.get(app_id)
        return WineSyncOption(game.sync) if game and game.sync is not None else WineSyncOption.AUTO

    def set_wine_sync(self, wine_sync: WineSyncOption, app_id) -> MangoHudLevel:
        """Set level for game launch option"""
        game = CONFIGURATION.games.get(app_id)
        game.sync = wine_sync.value if wine_sync is not None else WineSyncOption.AUTO.value
        CONFIGURATION.save_config()

    def get_prefered_gpu(self, app_id) -> GpuBrand | None:
        """Get GPU from game launch option"""
        game = CONFIGURATION.games.get(app_id)
        return GpuBrand(game.gpu) if game and game.gpu is not None else None

    def set_prefered_gpu(self, gpu_brand: GpuBrand, app_id) -> MangoHudLevel:
        """Set gpu for game launch option"""
        game = CONFIGURATION.games.get(app_id)
        game.gpu = gpu_brand.value if gpu_brand is not None else None
        CONFIGURATION.save_config()

    def get_environment(self, app_id) -> str:
        """Get defined env for game"""
        game = CONFIGURATION.games.get(app_id)
        return game.env if game.env is not None else ""

    def set_environment(self, env: str, app_id) -> MangoHudLevel:
        """Set gpu for game launch option"""
        game = CONFIGURATION.games.get(app_id)
        game.env = env
        CONFIGURATION.save_config()

    def get_parameters(self, app_id) -> str:
        """Get defined params for game"""
        game = CONFIGURATION.games.get(app_id)
        return game.args if game.args is not None else ""

    def is_proton(self, app_id) -> bool:
        """Check if app_id requires proton"""
        return CONFIGURATION.games.get(app_id).proton

    def set_parameters(self, params: str, app_id) -> MangoHudLevel:
        """Set gpu for game launch option"""
        game = CONFIGURATION.games.get(app_id)
        game.args = params
        CONFIGURATION.save_config()

    def is_steamdeck(self, app_id):
        """Check if app_id simulates steamdeck"""
        return CONFIGURATION.games.get(app_id).steamdeck

    def set_steamdeck(self, steamdeck: bool, app_id) -> MangoHudLevel:
        """Set gpu for game launch option"""
        game = CONFIGURATION.games.get(app_id)
        game.steamdeck = steamdeck
        CONFIGURATION.save_config()

    def get_run_configuration(self, app_id):
        "Get environment and wrappers for app_id"
        environment = {}
        wrappers = []
        params = ""

        game = CONFIGURATION.games.get(app_id)
        if game is not None:
            environment["SteamDeck"] = "1" if game.steamdeck else "0"

            if game.proton:
                if DEV_MODE:
                    environment["PROTON_LOG"] = "1"

                mode = WineSyncOption(game.sync)
                if mode == WineSyncOption.NTSYNC:
                    environment["PROTON_NO_NTSYNC"] = "0"
                    environment["PROTON_NO_FSYNC"] = "1"
                    environment["PROTON_NO_ESYNC"] = "1"
                elif mode == WineSyncOption.FSYNC:
                    environment["PROTON_NO_NTSYNC"] = "1"
                    environment["PROTON_NO_FSYNC"] = "0"
                    environment["PROTON_NO_ESYNC"] = "1"
                elif mode == WineSyncOption.ESYNC:
                    environment["PROTON_NO_NTSYNC"] = "1"
                    environment["PROTON_NO_FSYNC"] = "1"
                    environment["PROTON_NO_ESYNC"] = "0"
                elif mode == WineSyncOption.NONE:
                    environment["PROTON_NO_NTSYNC"] = "1"
                    environment["PROTON_NO_FSYNC"] = "1"
                    environment["PROTON_NO_ESYNC"] = "1"

            if game.gpu is not None:
                environment.update(HARDWARE_SERVICE.get_gpu_selector_env(GpuBrand(game.gpu)))

            if self.metrics_enabled:
                environment["MANGOHUD_CONFIG"] = f"preset={game.metrics_level}"
                environment["MANGOHUD_DLSYM"] = "1"
                environment["MANGOHUD"] = "1"
                wrappers.append("mangohud")

            if game.env is not None and len(game.env.strip()):
                env = game.env.strip()
                parts = env.split(" ")
                for part in parts:
                    env_parts = part.split("=")
                    environment[env_parts[0]] = env_parts[1]

            if game.args is not None:
                params = game.args.strip()

        self._logger.info(f"Config for {game.name}:")
        self._logger.info(f"  Environment: {environment}")
        self._logger.info(f"  Wrappers: {wrappers}")

        return environment, wrappers, params


STEAM_SERVICE = SteamService()
