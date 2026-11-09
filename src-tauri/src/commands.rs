//! Tauri commands used by the frontend.
//! `src/utils/backend/client.ts` is generated from these signatures by `tauri-rs-ts-ipc`.

use std::collections::{BTreeMap, HashMap};

use tauri::AppHandle;

use crate::{
    dto::{
        app::{AppState, Bootstrap, BrightnessOption},
        fans::{FanCurveSet, FanCurves},
        games::{GameConfigDetails, GameConfigEntry, GameListItem, GameSuggestion},
    },
    features::features,
    models::{
        hardware::{BatteryThreshold, FanCurveData, RgbBrightness},
        performance::PerformanceProfile,
        settings::GameEntry,
    },
    services::{application, hardware, openrgb, performance, steam},
    utils::{self, constants, shell::Shell, translator},
};

/// Runs blocking service code outside the main/async threads.
async fn blocking<T: Send + 'static>(f: impl FnOnce() -> T + Send + 'static) -> Result<T, String> {
    tauri::async_runtime::spawn_blocking(f).await.map_err(|e| e.to_string())
}

#[tauri::command]
pub fn get_bootstrap() -> Bootstrap {
    Bootstrap {
        status: crate::status(),
        version: constants::APP_VERSION.to_string(),
        app_name: constants::APP_NAME.to_string(),
        language: translator::language().to_string(),
        translations: translator::all(),
    }
}

/// Validates and stores the sudo password, resuming the initialization. Returns `false` on a wrong password.
#[tauri::command]
pub async fn submit_password(password: String) -> Result<bool, String> {
    blocking(move || {
        if !Shell::validate_password(&password) {
            return false;
        }
        if let Err(e) = crate::services::configuration::set_password(&password) {
            log::error!(target: "Main", "{e}");
        }
        crate::provide_password(password);
        true
    })
    .await
}

#[tauri::command]
pub async fn get_state() -> Result<AppState, String> {
    blocking(|| {
        let perf = performance::get();
        let rgb = openrgb::get();
        let hw = hardware::get();
        AppState {
            profile: perf.performance_profile().as_str().to_string(),
            profiles: PerformanceProfile::ALL.iter().map(|p| p.as_str().to_string()).collect(),
            scheduler: perf.current_scheduler(),
            schedulers: perf.available_schedulers(),
            ssd_scheduler: perf.current_ssd_scheduler(),
            ssd_schedulers: perf.available_ssd_schedulers(),
            fan_control: features().fan_control,
            effect: rgb.current_effect(),
            effects: rgb.available_effects(),
            brightness: rgb.current_brightness().value(),
            brightnesses: RgbBrightness::ALL
                .iter()
                .map(|b| BrightnessOption {
                    value: b.value(),
                    name: b.name().to_string(),
                })
                .collect(),
            color: rgb.color(),
            charge_limit: hw.charge_threshold().map(|t| t.value()),
            charge_limits: BatteryThreshold::ALL.iter().map(|t| t.value()).collect(),
            boot_sound: hw.boot_sound(),
            start_minimized: application::is_start_minimized(),
            autostart: application::is_autostart_enabled(),
            running_games: steam::get().running_games().len() as u32,
            on_battery: hw.on_battery(),
            dev_mode: *constants::DEV_MODE,
        }
    })
    .await
}

#[tauri::command]
pub async fn set_profile(profile: String) -> Result<(), String> {
    let profile = PerformanceProfile::from_str(&profile).ok_or_else(|| format!("Invalid profile '{profile}'"))?;
    blocking(move || {
        let perf = performance::get();
        if steam::get().running_games().is_empty() && perf.performance_profile() != profile {
            perf.set_performance_profile(profile, false, false, true);
        }
    })
    .await
}

#[tauri::command]
pub async fn set_scheduler(scheduler: String) -> Result<(), String> {
    blocking(move || {
        let perf = performance::get();
        if steam::get().running_games().is_empty() && scheduler != perf.current_scheduler() {
            perf.set_scheduler(&scheduler, false);
        }
    })
    .await
}

#[tauri::command]
pub async fn set_ssd_scheduler(scheduler: String) -> Result<(), String> {
    blocking(move || {
        let perf = performance::get();
        if steam::get().running_games().is_empty() && Some(scheduler.clone()) != perf.current_ssd_scheduler() {
            perf.set_ssd_scheduler(&scheduler, false);
        }
    })
    .await
}

#[tauri::command]
pub async fn set_effect(effect: String) -> Result<(), String> {
    blocking(move || openrgb::get().set_effect(&effect, false)).await
}

#[tauri::command]
pub async fn set_brightness(brightness: u32) -> Result<(), String> {
    let level = RgbBrightness::try_from(brightness)?;
    blocking(move || openrgb::get().set_brightness(level)).await
}

#[tauri::command]
pub async fn set_color(color: String) -> Result<(), String> {
    blocking(move || openrgb::get().set_color(&color)).await
}

#[tauri::command]
pub async fn set_charge_limit(limit: u32) -> Result<(), String> {
    let threshold = BatteryThreshold::try_from(limit)?;
    blocking(move || hardware::get().set_charge_threshold(threshold)).await?
}

#[tauri::command]
pub async fn set_boot_sound(enabled: bool) -> Result<(), String> {
    blocking(move || hardware::get().set_boot_sound(enabled)).await?
}

#[tauri::command]
pub async fn set_autostart(enabled: bool) -> Result<(), String> {
    blocking(move || application::set_autostart(enabled)).await
}

#[tauri::command]
pub async fn set_start_minimized(enabled: bool) -> Result<(), String> {
    blocking(move || application::set_start_minimized(enabled)).await
}

// ---------------------------------------------------------------------------------------------------------------------
// Games
// ---------------------------------------------------------------------------------------------------------------------

#[tauri::command]
pub async fn get_games() -> Result<Vec<GameListItem>, String> {
    blocking(|| {
        let svc = steam::get();
        let mut games: Vec<GameListItem> = svc
            .games()
            .into_iter()
            .map(|(id, g)| GameListItem {
                id,
                name: g.name,
                running: svc.is_running(id),
                icon: svc.icon(id).and_then(|p| utils::base64::image_data_url(&p)),
            })
            .collect();
        games.sort_by(|a, b| a.name.cmp(&b.name));
        games
    })
    .await
}

#[tauri::command]
pub async fn get_game_config(id: u32) -> Result<GameConfigDetails, String> {
    blocking(move || {
        let svc = steam::get();
        let perf = performance::get();
        let mut entry = svc.games().remove(&id).ok_or_else(|| format!("Game {id} not configured"))?;
        if entry.scheduler.is_none() {
            entry.scheduler = Some(perf.default_scheduler());
        }
        Ok(GameConfigDetails {
            id,
            entry: entry.into(),
            banner: svc.banner(id).and_then(|p| utils::base64::image_data_url(&p)),
            gpus: hardware::get().gpus().keys().cloned().collect(),
            schedulers: perf.available_schedulers(),
            default_scheduler: perf.default_scheduler(),
            metrics_enabled: svc.metrics_enabled(),
            metrics_levels: GameConfigDetails::metrics_levels(),
            devices: GameConfigDetails::devices(),
            sync_options: GameConfigDetails::sync_options(features().ntsync),
        })
    })
    .await?
}

#[tauri::command]
pub async fn get_game_suggestion(id: u32) -> Result<Option<GameSuggestion>, String> {
    blocking(move || {
        let yaml = utils::net::fetch(constants::SUGGESTIONS_URL).ok()?;
        let all: noyalib::Value = noyalib::from_str(&yaml).ok()?;
        let value = all.get(id.to_string().as_str()).or_else(|| all.get(id as usize))?;
        let entry = GameEntry::from_value(value)?;
        Some(GameSuggestion {
            args: entry.args,
            env: entry.env,
        })
    })
    .await
}

#[tauri::command]
pub async fn save_game_config(id: u32, entry: GameConfigEntry, launch: bool) -> Result<(), String> {
    blocking(move || {
        let svc = steam::get();
        let entry: GameEntry = entry.into();
        let launch_id = entry.overlay_id.clone().unwrap_or_else(|| id.to_string());
        svc.save_game_config(id, entry);
        if launch {
            svc.launch_game(&launch_id);
        }
    })
    .await
}

#[tauri::command]
pub async fn launch_game(id: u32) -> Result<(), String> {
    blocking(move || {
        let svc = steam::get();
        let launch_id = svc
            .games()
            .get(&id)
            .and_then(|g| g.overlay_id.clone())
            .unwrap_or_else(|| id.to_string());
        svc.launch_game(&launch_id);
    })
    .await
}

#[tauri::command]
pub async fn answer_decky_install(accept: bool) -> Result<(), String> {
    blocking(move || {
        if accept {
            steam::get().install_rccdc(true);
        } else {
            steam::get().decline_rccdc();
        }
    })
    .await
}

// ---------------------------------------------------------------------------------------------------------------------
// Fan curves
// ---------------------------------------------------------------------------------------------------------------------

#[tauri::command]
pub async fn get_fan_curves() -> Result<FanCurves, String> {
    blocking(|| {
        let perf = performance::get();
        let mut fans = perf.fans();
        fans.sort();
        let mut curves = BTreeMap::new();
        for profile in PerformanceProfile::ALL.iter().filter(|p| **p != PerformanceProfile::Smart) {
            let per_fan = fans
                .iter()
                .map(|fan| {
                    let mut defaults = perf.default_fan_curve(fan, profile.as_str());
                    defaults.normalize();
                    (
                        fan.clone(),
                        FanCurveSet {
                            current: perf.fan_curve(fan, profile.as_str()),
                            defaults,
                        },
                    )
                })
                .collect();
            curves.insert(profile.as_str().to_string(), per_fan);
        }
        FanCurves {
            fans,
            selected_profile: perf.performance_profile().as_str().to_string(),
            curves,
        }
    })
    .await
}

/// `curves`: profile -> fan -> curve
#[tauri::command]
pub async fn save_fan_curves(curves: BTreeMap<String, HashMap<String, FanCurveData>>) -> Result<(), String> {
    blocking(move || performance::get().save_fan_curves(curves)).await
}

// ---------------------------------------------------------------------------------------------------------------------
// Application
// ---------------------------------------------------------------------------------------------------------------------

#[tauri::command]
pub fn hide_window(app: AppHandle) {
    crate::hide_main_window(&app);
}

#[tauri::command]
pub async fn quit(app: AppHandle) -> Result<(), String> {
    blocking(move || crate::quit(&app)).await
}
