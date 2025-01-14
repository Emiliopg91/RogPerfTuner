import subprocess
import os
import json
import time
import decky  # eslint: disable=E0401
from plugin_config import PluginConfig
from plugin_logger import PluginLogger


class Plugin:
    # Configuration

    async def get_config(self):
        decky.logger.debug("Executing: get_config()")
        return PluginConfig.get_config()

    async def set_config(self, key: str, value):
        decky.logger.debug("Executing: set_config(%s, %s)", key, str(value))
        PluginConfig.set_config(key, value)

    # Logger

    async def log(self, level: str, msg: str) -> int:
        return PluginLogger.log(level, msg)

    async def get_plugin_log(self) -> str:
        decky.logger.debug("Executing: get_plugin_log()")
        return PluginLogger.get_plugin_log()

    # Lifecycle

    async def _main(self):
        PluginLogger.configure_logger()
        decky.logger.info("Running " + decky.DECKY_PLUGIN_NAME)
        folder_path = decky.DECKY_PLUGIN_RUNTIME_DIR
        for file_name in os.listdir(folder_path):
            file_path = os.path.join(folder_path, file_name)
            # Verificar si es un archivo antes de intentar eliminarlo
            if os.path.isfile(file_path):
                os.remove(file_path)

    async def _unload(self):
        decky.logger.info("Unloading " + decky.DECKY_PLUGIN_NAME)

    async def _migration(self):
        decky.logger.info("Migrating plugin configuration")
        PluginConfig.migrate()

    async def dbus_launch_game(self, game_name: str):
        decky.logger.info(f"Running dbus_launch_game({game_name})")
        events = [game_name]

        # Convertir a JSON
        json_data = json.dumps(events, indent=4)

        # Generar el nombre del archivo con el timestamp actual
        timestamp = int(time.time())
        file_name = os.path.join(decky.DECKY_PLUGIN_RUNTIME_DIR, f"launch-{timestamp}.event")

        # Escribir el JSON en el archivo
        with open(file_name, "w") as file:
            file.write(json_data)

    async def dbus_stop_game(self, game_name: str):
        decky.logger.info(f"Running dbus_stop_game({game_name})")
        events = [game_name]

        # Convertir a JSON
        json_data = json.dumps(events, indent=4)

        # Generar el nombre del archivo con el timestamp actual
        timestamp = int(time.time())
        file_name = os.path.join(decky.DECKY_PLUGIN_RUNTIME_DIR, f"stop-{timestamp}.event")

        # Escribir el JSON en el archivo
        with open(file_name, "w") as file:
            file.write(json_data)
