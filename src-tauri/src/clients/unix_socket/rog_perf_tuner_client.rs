//! Running instance of this application, used by the CLI options.

use noyalib::Value;

use crate::{
    clients::unix_socket::invoke_once,
    models::{
        socket::{int_value, str_value, value_to_string},
        steam::SteamGameConfig,
    },
    utils::constants,
};

fn invoke(method: &str, data: Vec<Value>) -> Result<Vec<Value>, String> {
    invoke_once(&constants::SOCKET_FILE, method, data, 3000)
}

fn invoke_str(method: &str) -> Result<String, String> {
    invoke(method, vec![]).map(|d| d.first().map(value_to_string).unwrap_or_default())
}

pub fn next_profile() -> Result<String, String> {
    invoke_str(constants::PERF_PROF)
}

pub fn next_effect() -> Result<String, String> {
    invoke_str(constants::NEXT_EFF)
}

pub fn increase_brightness() -> Result<String, String> {
    invoke_str(constants::INC_BRIGHT)
}

pub fn decrease_brightness() -> Result<String, String> {
    invoke_str(constants::DEC_BRIGHT)
}

pub fn show_gui() -> Result<(), String> {
    invoke(constants::SHOW_GUI, vec![]).map(|_| ())
}

pub fn game_config(steam_id: &str) -> Result<SteamGameConfig, String> {
    let id = steam_id.trim().parse::<i64>().map(int_value).unwrap_or_else(|_| str_value(steam_id));
    let res = invoke(constants::GAME_CFG, vec![id])?;
    let yaml = res.first().map(value_to_string).ok_or("Empty response")?;
    noyalib::from_str(&yaml).map_err(|e| e.to_string())
}

#[cfg(test)]
mod tests {
    /// Read-only request against a running instance: `cargo test -- --ignored`
    #[test]
    #[ignore]
    fn game_config_from_running_instance() {
        let cfg = super::game_config("1091500").expect("request failed");
        println!("{cfg:?}");
        assert!(cfg.wrappers.iter().any(|w| w.contains("systemd-inhibit")) || !cfg.environment.is_empty());
    }
}
