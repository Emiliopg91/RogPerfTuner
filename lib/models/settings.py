from dataclasses import dataclass


@dataclass
class Effect:
    """Configuration helper class"""

    brightness: int
    color: str = None


@dataclass
class OpenRgb:
    """Configuration helper class"""

    last_effect: str
    effects: dict[str, Effect]


@dataclass
class PlatformProfiles:
    """Configuration helper class"""

    last: int
    boost: int


@dataclass
class Platform:
    """Configuration helper class"""

    profiles: PlatformProfiles


@dataclass
class Settings:
    """Configuration helper class"""

    password: str


@dataclass
class Config:
    """Configuration helper class"""

    settings: Settings
    platform: Platform
    open_rgb: OpenRgb
