from dataclasses import dataclass, field

from dataclasses_json import DataClassJsonMixin

from rcc.models.mangohud_level import MangoHudLevel
from rcc.models.wine_sync_option import WineSyncOption


@dataclass
class Effect(DataClassJsonMixin):
    """Configuration helper class"""

    color: str | None = None


@dataclass
class OpenRgb(DataClassJsonMixin):
    """Configuration helper class"""

    brightness: int
    last_effect: str
    effects: dict[str, Effect]


@dataclass
class PlatformProfiles(DataClassJsonMixin):
    """Configuration helper class"""

    profile: int


@dataclass
class Platform(DataClassJsonMixin):
    """Configuration helper class"""

    profiles: PlatformProfiles


@dataclass
class Settings(DataClassJsonMixin):
    """Configuration helper class"""

    password: str


@dataclass
class GameEntry(DataClassJsonMixin):
    """Game configuration"""

    name: str
    gpu: str | None = field(default=None)
    metrics_level: int = field(default=MangoHudLevel.NO_DISPLAY.value)
    sync: str = field(default=WineSyncOption.AUTO.value)
    env: str = field(default=None)
    args: str = field(default=None)
    proton: bool = field(default=True)


@dataclass
class Config(DataClassJsonMixin):
    """Configuration helper class"""

    logger: dict[str, str]
    settings: Settings
    platform: Platform
    open_rgb: OpenRgb
    games: dict[int, GameEntry]
