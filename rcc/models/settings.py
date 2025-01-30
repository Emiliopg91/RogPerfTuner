from dataclasses import dataclass

from dataclasses_json import dataclass_json


@dataclass_json
@dataclass
class Effect:
    """Configuration helper class"""

    color: str | None = None


@dataclass_json
@dataclass
class OpenRgb:
    """Configuration helper class"""

    brightness: int
    last_effect: str
    effects: dict[str, Effect]


@dataclass_json
@dataclass
class PlatformProfiles:
    """Configuration helper class"""

    profile: int


@dataclass_json
@dataclass
class Platform:
    """Configuration helper class"""

    profiles: PlatformProfiles


@dataclass_json
@dataclass
class Settings:
    """Configuration helper class"""

    password: str


@dataclass_json
@dataclass
class GameEntry:
    """Game configuration"""

    name: str
    profile: int


@dataclass_json
@dataclass
class Config:
    """Configuration helper class"""

    logger: dict[str, str]
    settings: Settings
    platform: Platform
    open_rgb: OpenRgb
    games: dict[int, GameEntry]
