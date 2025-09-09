import os
import json
import time
import decky  # eslint: disable=E0401
from plugin_config import PluginConfig
from plugin_logger import PluginLogger
from utils.rcc_websocket import WebsocketServer


class Plugin:
    def __init__(self):
        self.server = None

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

    async def _unload(self):
        decky.logger.info("Unloading " + decky.DECKY_PLUGIN_NAME)

    async def _migration(self):
        decky.logger.info("Migrating plugin configuration")
        PluginConfig.migrate()

    async def emit_event(self, name: str, *args: any):
        decky.logger.info(f"Running emit_event({name},{tuple(args)})")
        self.server.emit(name, *args)

    async def send_response(self, id: str, method: str, *args: any):
        decky.logger.info(f'Running send_response("{id}","{method}",{tuple(args)})')
        self.server.send_response(id, method, *args)

    async def ready(self):
        decky.logger.info(f"Running ready()")
        self.server = WebsocketServer(18158)

    async def shutdown(self):
        decky.logger.info(f"Running shutdown()")
        self.server.shutdown()
