from dataclasses import dataclass, field


@dataclass
class SteamGameDetails:
    """Steam game details class"""

    appid: int
    name: str
    is_steam_app: bool = field(default=True)
    launch_opts: str | None = field(default="%command%")

    @property
    def gpu(self):
        """Dedicated gpu flag"""
        return "VK_ICD_FILENAMES" in self.launch_opts
