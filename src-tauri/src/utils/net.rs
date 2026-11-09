//! Network helpers, delegated to curl.

use std::fs;

use crate::utils::shell::{Shell, shell_quote};

pub fn download(url: &str, dst: &str) -> Result<(), String> {
    log::info!(target: "NetUtils", "Downloading {url} into {dst}");
    let res = Shell::get()
        .run_command(&format!("curl -fsSL -o {} {}", shell_quote(dst), shell_quote(url)), true)
        .map_err(|e| e.to_string());
    if res.is_err() {
        let _ = fs::remove_file(dst);
    }
    res.map(|_| ())
}

pub fn fetch(url: &str) -> Result<String, String> {
    log::info!(target: "NetUtils", "Fetching {url}");
    Shell::get()
        .run_command(&format!("curl -fsSL {}", shell_quote(url)), true)
        .map(|r| r.stdout)
        .map_err(|e| e.to_string())
}
