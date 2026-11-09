//! Games list and per game configuration.

use serde::{Deserialize, Serialize};

use crate::models::settings::{ComputerType, GameEntry, MangoHudLevel, WineSyncOption};

#[derive(Debug, Clone, Serialize)]
pub struct GameListItem {
    pub id: u32,
    pub name: String,
    pub running: bool,
    pub icon: Option<String>,
}

/// Editable game configuration. Enum values are sent as their config strings
/// (`no_display`, `steam_deck`, `ntsync`...), listed in `GameConfigDetails`.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameConfigEntry {
    pub name: String,
    pub args: Option<String>,
    pub env: Option<String>,
    pub gpu: Option<String>,
    pub scheduler: Option<String>,
    pub metrics_level: String,
    pub overlay_id: Option<String>,
    pub proton: bool,
    pub device: String,
    pub sync: String,
    pub wrappers: Option<String>,
}

fn enum_str<T: Serialize>(v: &T) -> String {
    serde_json::to_value(v)
        .ok()
        .and_then(|v| v.as_str().map(|s| s.to_string()))
        .unwrap_or_default()
}

fn enum_parse<T: for<'de> Deserialize<'de> + Default>(s: &str) -> T {
    serde_json::from_value(serde_json::Value::String(s.to_string())).unwrap_or_default()
}

impl From<GameEntry> for GameConfigEntry {
    fn from(e: GameEntry) -> Self {
        Self {
            name: e.name,
            args: e.args,
            env: e.env,
            gpu: e.gpu,
            scheduler: e.scheduler,
            metrics_level: enum_str(&e.metrics_level),
            overlay_id: e.overlay_id,
            proton: e.proton,
            device: enum_str(&e.device),
            sync: enum_str(&e.sync),
            wrappers: e.wrappers,
        }
    }
}

impl From<GameConfigEntry> for GameEntry {
    fn from(e: GameConfigEntry) -> Self {
        Self {
            name: e.name,
            args: e.args,
            env: e.env,
            gpu: e.gpu.filter(|g| !g.is_empty()),
            scheduler: e.scheduler.filter(|s| !s.is_empty()),
            metrics_level: enum_parse::<MangoHudLevel>(&e.metrics_level),
            overlay_id: e.overlay_id,
            proton: e.proton,
            device: enum_parse::<ComputerType>(&e.device),
            sync: enum_parse::<WineSyncOption>(&e.sync),
            wrappers: e.wrappers,
        }
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct GameConfigDetails {
    pub id: u32,
    pub entry: GameConfigEntry,
    pub banner: Option<String>,
    pub gpus: Vec<String>,
    pub schedulers: Vec<String>,
    pub default_scheduler: String,
    pub metrics_enabled: bool,
    pub metrics_levels: Vec<String>,
    pub devices: Vec<String>,
    pub sync_options: Vec<String>,
}

impl GameConfigDetails {
    pub fn metrics_levels() -> Vec<String> {
        MangoHudLevel::ALL.iter().map(enum_str).collect()
    }

    pub fn devices() -> Vec<String> {
        [ComputerType::Computer, ComputerType::SteamDeck].iter().map(enum_str).collect()
    }

    pub fn sync_options(ntsync: bool) -> Vec<String> {
        [
            WineSyncOption::Auto,
            WineSyncOption::Ntsync,
            WineSyncOption::Fsync,
            WineSyncOption::Esync,
            WineSyncOption::None,
        ]
        .iter()
        .filter(|o| ntsync || **o != WineSyncOption::Ntsync)
        .map(enum_str)
        .collect()
    }
}

/// Suggested `args`/`env` for a game from the shared suggestions file
#[derive(Debug, Clone, Serialize)]
pub struct GameSuggestion {
    pub args: Option<String>,
    pub env: Option<String>,
}

/// Payload of `open_game_config`
#[derive(Debug, Clone, Serialize)]
pub struct OpenGameConfig {
    pub id: u32,
    pub first_run: bool,
}
