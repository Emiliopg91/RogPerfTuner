from typing import Callable
from rcc.clients.websocket.base.abstract_websocket_client import AbstractWebsocketClient
from rcc.utils.singleton import singleton


@singleton
class SteamClient(AbstractWebsocketClient):
    """Steam websocket client"""

    def __init__(self):
        super().__init__(18158)

    def on_launch_game(self, callback: Callable[[int, str], None]):
        """Handler for launch game events"""
        self.on("launch_game", callback)

    def on_stop_game(self, callback: Callable[[int, str], None]):
        """Handler for stop game events"""
        self.on("stop_game", callback)

    def get_running_games(self) -> list:
        """Retreive Steam running games. Default empty list"""
        try:
            return self._invoke("get_running_games", timeout=0.5)[0]
        except Exception:
            self._logger.debug("Could not get running games, defaulting to []")
            return []


steam_client = SteamClient()
