// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::process::exit;

/// Binary entrypoint: handles the command line options or starts the GUI.
fn main() {
    if unsafe { libc::geteuid() } == 0 {
        eprintln!("This program must not be run as root (sudo). Please run it as a regular user.");
        exit(1);
    }

    #[cfg(debug_assertions)]
    {
        // Regenerates the TypeScript bindings (src/utils/backend) from the Rust sources
        if std::env::var("IN_DEBUG").is_err() && std::env::var("CARGO_MANIFEST_DIR").is_ok() {
            tauri_rs_ts_ipc::build();
        }
    }

    let args: Vec<String> = std::env::args().collect();
    if let Some(code) = rog_perf_tuner_lib::handle_cli(&args) {
        exit(code);
    }

    rog_perf_tuner_lib::run(&args[0]);
}
