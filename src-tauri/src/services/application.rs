//! Application level tasks: menu entry, helper scripts, autostart, shutdown.

use log::{error, info};

use crate::{
    services::configuration,
    utils::{
        self, constants,
        event_bus::{self, Event},
        logger::tabbed,
    },
};

const LOG: &str = "ApplicationService";

fn desktop_file() -> String {
    format!(
        "[Desktop Entry]\nExec={}\nIcon={}\nName={}\nWName={}\nComment=An utility to manage Asus Rog laptop performance\nPath=\nTerminal=False\nType=Application\nCategories=Utility;\n",
        constants::EXEC_NAME,
        *constants::ASSET_ICON_45_FILE,
        constants::APP_NAME,
        constants::APP_NAME
    )
}

fn script(exec_path: &str, option: &str) -> String {
    format!("#!/bin/bash\n\nset -e\n\n\"{exec_path}\" {option}\n")
}

fn wrapper_script(exec_path: &str, option: &str) -> String {
    format!(
        "#!/usr/bin/env bash\n\nset -e\n\nTARGET=\"{exec_path}\"\n\nif [[ -f \"$TARGET\" && -x \"$TARGET\" ]]; then\n    exec \"$TARGET\" {option} \"$@\"\nelse\n    exec \"$@\"\nfi\n"
    )
}

pub fn init(exec_path: &str) {
    info!(target: LOG, "Initializing ApplicationService");
    tabbed(|| {
        if !*constants::DEV_MODE && !utils::files::exists(&constants::APP_DRAW_FILE) {
            info!(target: LOG, "Creating menu entry");
            if let Err(e) = utils::files::write_file(&constants::APP_DRAW_FILE, &desktop_file()) {
                error!(target: LOG, "Couldn't create menu entry: {e}");
            }
        }

        info!(target: LOG, "Creating helper scripts");
        tabbed(|| {
            let _ = utils::files::remove_path(&constants::BIN_DIR);
            let files = [
                (&*constants::NEXT_PROFILE_PATH, script(exec_path, "-p")),
                (&*constants::NEXT_EFFECT_PATH, script(exec_path, "-e")),
                (&*constants::INCREASE_BRIGHTNESS_PATH, script(exec_path, "-i")),
                (&*constants::DECREASE_BRIGHTNESS_PATH, script(exec_path, "-d")),
                (&*constants::STEAM_WRAPPER_PATH, wrapper_script(exec_path, "-r")),
                (&*constants::FLATPAK_WRAPPER_PATH, wrapper_script(exec_path, "-f")),
                (&*constants::SHOW_GUI_PATH, wrapper_script(exec_path, "-s")),
            ];
            for (path, content) in files {
                if let Err(e) = utils::files::write_file(path, &content) {
                    error!(target: LOG, "Couldn't write {path}: {e}");
                }
            }
            let _ = utils::files::chmod_recursive(&constants::BIN_DIR, 0o777);
        });
    });
}

pub fn is_autostart_enabled() -> bool {
    utils::files::exists(&constants::AUTOSTART_FILE)
}

pub fn set_autostart(enabled: bool) {
    if enabled {
        if !is_autostart_enabled() && utils::files::write_file(&constants::AUTOSTART_FILE, &desktop_file()).is_ok() {
            info!(target: LOG, "Autostart file {} written successfully", *constants::AUTOSTART_FILE);
        }
    } else {
        let _ = utils::files::remove_path(&constants::AUTOSTART_FILE);
        info!(target: LOG, "Autostart file {} deleted successfully", *constants::AUTOSTART_FILE);
    }
}

pub fn is_start_minimized() -> bool {
    configuration::read(|c| c.application.start_minimized)
}

pub fn set_start_minimized(enabled: bool) {
    if enabled != is_start_minimized() {
        configuration::update(|c| c.application.start_minimized = enabled);
    }
}

pub fn shutdown() {
    info!(target: LOG, "Starting application shutdown");
    tabbed(|| event_bus::emit(Event::ApplicationShutdown));
    info!(target: LOG, "Shutdown finished");
    info!(target: LOG, "Stopping application");
    event_bus::emit(Event::ApplicationStop);
}
