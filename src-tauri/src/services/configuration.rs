//! Configuration storage: `~/.RogPerfTuner/config/config.yaml` plus the sudo
//! password kept in the desktop keyring.

use std::{
    fs,
    sync::{LazyLock, RwLock},
};

use log::{error, info};

use crate::{models::settings::RootConfig, utils::constants};

const LOG: &str = "ConfigurationWrapper";

static CONFIG: LazyLock<RwLock<RootConfig>> = LazyLock::new(|| RwLock::new(load_from_disk()));

fn config_path() -> String {
    format!("{}/{}", *constants::CONFIG_DIR, constants::CONFIG_FILE)
}

fn load_from_disk() -> RootConfig {
    let path = config_path();
    match fs::read_to_string(&path) {
        Ok(content) => noyalib::from_str(&content).unwrap_or_else(|e| {
            error!(target: LOG, "Error parsing {path}: {e}, using defaults");
            RootConfig::default()
        }),
        Err(_) => {
            info!(target: LOG, "No configuration found, creating defaults");
            let cfg = RootConfig::default();
            write_to_disk(&cfg);
            cfg
        }
    }
}

fn write_to_disk(cfg: &RootConfig) {
    let _ = fs::create_dir_all(constants::CONFIG_DIR.as_str());
    match noyalib::to_string(cfg) {
        Ok(yaml) => {
            if let Err(e) = fs::write(config_path(), yaml) {
                error!(target: LOG, "Error saving configuration: {e}");
            }
        }
        Err(e) => error!(target: LOG, "Error serializing configuration: {e}"),
    }
}

/// Reads from the configuration.
pub fn read<T>(f: impl FnOnce(&RootConfig) -> T) -> T {
    f(&CONFIG.read().unwrap())
}

/// Modifies the configuration and saves it.
pub fn update<T>(f: impl FnOnce(&mut RootConfig) -> T) -> T {
    let mut cfg = CONFIG.write().unwrap();
    let res = f(&mut cfg);
    write_to_disk(&cfg);
    res
}

/// Reloads the configuration from disk.
pub fn reload() {
    *CONFIG.write().unwrap() = load_from_disk();
}

fn keyring_entry() -> Option<keyring::Entry> {
    keyring::Entry::new(constants::EXEC_NAME, "password")
        .map_err(|e| error!(target: LOG, "Error accessing keyring: {e}"))
        .ok()
}

pub fn password() -> String {
    keyring_entry()
        .and_then(|e| match e.get_password() {
            Ok(p) => Some(p),
            Err(keyring::Error::NoEntry) => None,
            Err(e) => {
                error!(target: LOG, "Error on password fetching: {e}");
                None
            }
        })
        .unwrap_or_default()
}

pub fn set_password(pass: &str) -> Result<(), String> {
    keyring_entry()
        .ok_or("Keyring not available")?
        .set_password(pass)
        .map_err(|e| format!("Error saving password: {e}"))
}
