//! Events sent to the frontend. Every `emit` lives here, with literal event names and typed payloads,
//! so `tauri-rs-ts-ipc` can generate `src/utils/backend/listener.ts` from them.

use tauri::{AppHandle, Emitter};

use crate::dto::{app::InitStatus, games::OpenGameConfig};

/// The backend state changed (profile, effect, battery...), the frontend should reload it.
pub fn emit_state_changed(app: &AppHandle) {
    let _ = app.emit("state_changed", ());
}

pub fn emit_status_changed(app: &AppHandle, status: InitStatus) {
    let _ = app.emit("status_changed", status);
}

/// Opens one of the dialogs: `games` or `fans`.
pub fn emit_show_view(app: &AppHandle, view: String) {
    let _ = app.emit("show_view", view);
}

pub fn emit_open_game_config(app: &AppHandle, payload: OpenGameConfig) {
    let _ = app.emit("open_game_config", payload);
}

/// Asks whether the Decky plugin should be installed.
pub fn emit_ask_decky_install(app: &AppHandle) {
    let _ = app.emit("ask_decky_install", ());
}

/// The list of games changed (a game started or stopped, or a new one was configured).
pub fn emit_games_changed(app: &AppHandle) {
    let _ = app.emit("games_changed", ());
}
