//! Application state and initialization.

use std::collections::HashMap;

use serde::{Deserialize, Serialize};

/// Initialization status of the backend
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum InitStatus {
    Loading,
    Password,
    MissingModule,
    Ready,
}

#[derive(Debug, Clone, Serialize)]
pub struct Bootstrap {
    pub status: InitStatus,
    pub version: String,
    pub app_name: String,
    pub language: String,
    pub translations: HashMap<String, String>,
}

#[derive(Debug, Clone, Serialize)]
pub struct BrightnessOption {
    pub value: u32,
    pub name: String,
}

/// Everything the main window shows. Profiles are sent as lower case strings (`smart`, `performance`...).
#[derive(Debug, Clone, Serialize)]
pub struct AppState {
    pub profile: String,
    pub profiles: Vec<String>,
    pub scheduler: String,
    pub schedulers: Vec<String>,
    pub ssd_scheduler: Option<String>,
    pub ssd_schedulers: Vec<String>,
    pub fan_control: bool,
    pub effect: String,
    pub effects: Vec<String>,
    pub brightness: u32,
    pub brightnesses: Vec<BrightnessOption>,
    pub color: Option<String>,
    pub charge_limit: Option<u32>,
    pub charge_limits: Vec<u32>,
    pub boot_sound: Option<bool>,
    pub start_minimized: bool,
    pub autostart: bool,
    pub running_games: u32,
    pub on_battery: bool,
    pub dev_mode: bool,
}
