import os
import subprocess
from threading import Thread

import psutil
from lib.clients.file.steam_client import steam_client
from lib.models.settings import Game
from lib.models.thermal_throttle_profile import ThermalThrottleProfile
from lib.services.platform_service import platform_service
from lib.utils.configuration import configuration
from lib.utils.constants import rccdc_asset_path, user_plugin_folder
from lib.utils.cryptography import cryptography
from lib.utils.logger import Logger


class SteamService:
    """Steam service"""

    def __init__(self):
        self._logger = Logger()
        self._rccdc_enabled = False
        steam_client.on("launch", self.launch_game)
        steam_client.on("stop", self.stop_game)

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
                self.__copy_plugin(rccdc_asset_path, plugins_folder, False)
            else:
                if os.path.getmtime(rccdc_path) < os.path.getmtime(rccdc_asset_path):
                    self._logger.info("Updating plugin")
                    self.__copy_plugin(rccdc_asset_path, plugins_folder, True)
                else:
                    self._logger.info("Plugin up to date")
            self._rccdc_enabled = True
        else:
            self._logger.warning("No Decky installation found, skipping")
            self._rccdc_enabled = False

    def __copy_plugin(self, src: str, dst: str, is_update: bool):
        subprocess.run(
            ["cp", "-R", src, user_plugin_folder],
            text=True,
            check=True,
        )
        if is_update:
            subprocess.run(
                ["sudo", "-S", "rm", "-R", dst],
                input=cryptography.decrypt_string(configuration.settings.password) + "\n",
                text=True,
                check=True,
            )
        subprocess.run(
            ["sudo", "-S", "cp", "-R", os.path.join(user_plugin_folder, "RCCDeckyCompanion"), dst],
            input=cryptography.decrypt_string(configuration.settings.password) + "\n",
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
        self._logger.add_tab()
        if game not in configuration.games:
            configuration.games[game] = Game(ThermalThrottleProfile.PERFORMANCE.value)
            configuration.save_config()

        profile = ThermalThrottleProfile(configuration.games[game].profile)
        platform_service.set_thermal_throttle_policy(profile, True, game, True)
        self._logger.rem_tab()

    def stop_game(self, game: str):
        """Restore profile after game stop"""
        self._logger.info(f"Stopped {game}")
        self._logger.add_tab()
        platform_service.restore_profile()
        self._logger.rem_tab()


steam_service = SteamService()
