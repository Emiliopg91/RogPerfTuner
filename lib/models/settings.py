from dataclasses import dataclass, field
from typing import Dict, Any

@dataclass
class Effect:
    brightness: int
    color: str = None

@dataclass
class OpenRgb:
    lastEffect: str
    effects: dict[str, Effect]

@dataclass
class PlatformProfiles:
    last: int

@dataclass
class Platform:
    profiles: PlatformProfiles

@dataclass
class Settings:
    password: str

@dataclass
class Config:
    settings: Settings
    platform: Platform
    openRgb: OpenRgb