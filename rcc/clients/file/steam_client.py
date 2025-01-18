import os
from rcc.clients.file.base.abstract_file_client import AbstractFileClient


class SteamClient(AbstractFileClient):
    """Steam file client"""

    def __init__(self):
        super().__init__(os.path.expanduser(os.path.join("~", "homebrew", "data", "RCCDeckyCompanion")))


steam_client = SteamClient()
