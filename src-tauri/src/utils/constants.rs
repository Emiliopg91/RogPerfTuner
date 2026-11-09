//! Application wide constants.

use std::{env, sync::LazyLock};

pub const APP_NAME: &str = "RogPerfTuner";
pub const EXEC_NAME: &str = "rog-perf-tuner";
pub const APP_VERSION: &str = env!("RPT_APP_VERSION");
pub const PLUGIN_VERSION: &str = env!("RPT_PLUGIN_VERSION");

pub const LOG_FILE_NAME: &str = APP_NAME;
pub const LOG_RUNNER_FILE_NAME: &str = "Runner";
pub const LOG_ORGB_FILE_NAME: &str = "OpenRGB";
pub const CONFIG_FILE: &str = "config.yaml";

// Socket method names (shared with the helper scripts and the Decky plugin)
pub const PERF_PROF: &str = "nexPerformanceProfile";
pub const DEC_BRIGHT: &str = "decRgbBrightness";
pub const INC_BRIGHT: &str = "incRgbBrightness";
pub const NEXT_EFF: &str = "nextRgbEffect";
pub const SHOW_GUI: &str = "showGui";
pub const GAME_CFG: &str = "gameConfig";

pub const USR_SHARE_OCL_DIR: &str = "/etc/OpenCL/vendors/";
pub const USR_SHARE_VK_DIR: &str = "/usr/share/vulkan/icd.d/";
pub const FLATPAK_MANGOHUD: &str = "org.freedesktop.Platform.VulkanLayer.MangoHud/x86_64/25.08";
pub const FLATPAK_MANGOHUD_OVERRIDE: &str = "--filesystem=xdg-config/MangoHud:ro";
pub const SUGGESTIONS_URL: &str =
    "https://raw.githubusercontent.com/Emiliopg91/RogPerfTuner/refs/heads/main/resources/suggestions.yaml";

pub static PID: LazyLock<u32> = LazyLock::new(std::process::id);

pub static DEV_MODE: LazyLock<bool> =
    LazyLock::new(|| cfg!(debug_assertions) || env::var("DEV_MODE").is_ok());

pub static HOME_DIR: LazyLock<String> = LazyLock::new(|| env::var("HOME").unwrap_or_default());
pub static XDG_RUNTIME_DIR: LazyLock<String> = LazyLock::new(|| {
    env::var("XDG_RUNTIME_DIR").unwrap_or_else(|_| format!("/run/user/{}", unsafe { libc::getuid() }))
});

fn app_home(sub: &str) -> String {
    format!("{}/.{}/{}", *HOME_DIR, APP_NAME, sub)
}

/// Root of the repository when running from sources (dev mode).
fn repo_dir() -> String {
    let manifest = env!("CARGO_MANIFEST_DIR");
    std::path::Path::new(manifest)
        .parent()
        .map(|p| p.display().to_string())
        .unwrap_or_default()
}

pub static ASSETS_DIR: LazyLock<String> = LazyLock::new(|| {
    if *DEV_MODE {
        format!("{}/build/assets", repo_dir())
    } else {
        format!("/usr/share/{EXEC_NAME}")
    }
});
pub static APP_DRAW_FILE: LazyLock<String> = LazyLock::new(|| {
    if *DEV_MODE {
        format!("{}/resources/{APP_NAME}.desktop", repo_dir())
    } else {
        format!("/usr/share/applications/{EXEC_NAME}.desktop")
    }
});
pub static UDEV_RULES: LazyLock<String> = LazyLock::new(|| {
    if *DEV_MODE {
        format!("{}/OpenRGB/60-openrgb.rules", *ASSETS_DIR)
    } else {
        format!("/usr/lib/udev/rules.d/60-{EXEC_NAME}.rules")
    }
});

pub static LOCK_FILE: LazyLock<String> = LazyLock::new(|| format!("{}/{APP_NAME}.lock", *XDG_RUNTIME_DIR));
pub static SOCKET_FILE: LazyLock<String> = LazyLock::new(|| format!("{}/{APP_NAME}.sock", *XDG_RUNTIME_DIR));
pub static RCCDC_SOCKET_PATH: LazyLock<String> =
    LazyLock::new(|| format!("{}/RCCDeckyCompanion.sock", *XDG_RUNTIME_DIR));

pub static ASSET_ICON_FILE: LazyLock<String> = LazyLock::new(|| format!("{}/icons/icon.svg", *ASSETS_DIR));
pub static ASSET_ICON_45_FILE: LazyLock<String> = LazyLock::new(|| format!("{}/icons/icon-45x45.png", *ASSETS_DIR));
pub static ORGB_PATH: LazyLock<String> = LazyLock::new(|| format!("{}/OpenRGB/openrgb", *ASSETS_DIR));
pub static DEV_MODE_FILE: LazyLock<String> = LazyLock::new(|| format!("{}/dev-mode.sh", *ASSETS_DIR));
pub static RCCDC_ASSET_PATH: LazyLock<String> = LazyLock::new(|| format!("{}/RccDeckyCompanion", *ASSETS_DIR));

pub static BIN_DIR: LazyLock<String> = LazyLock::new(|| app_home("bin"));
pub static SHOW_GUI_PATH: LazyLock<String> = LazyLock::new(|| app_home("bin/application/showGui"));
pub static NEXT_PROFILE_PATH: LazyLock<String> = LazyLock::new(|| app_home("bin/performance/nextProfile"));
pub static DECREASE_BRIGHTNESS_PATH: LazyLock<String> = LazyLock::new(|| app_home("bin/rgb/decreaseBrightness"));
pub static INCREASE_BRIGHTNESS_PATH: LazyLock<String> = LazyLock::new(|| app_home("bin/rgb/increaseBrightness"));
pub static NEXT_EFFECT_PATH: LazyLock<String> = LazyLock::new(|| app_home("bin/rgb/nextEffect"));
pub static BIN_STEAM_DIR: LazyLock<String> = LazyLock::new(|| app_home("bin/steam"));
pub static STEAM_WRAPPER_PATH: LazyLock<String> = LazyLock::new(|| app_home("bin/steam/run"));
pub static FLATPAK_WRAPPER_PATH: LazyLock<String> = LazyLock::new(|| app_home("bin/steam/flatpak"));
pub static CONFIG_DIR: LazyLock<String> = LazyLock::new(|| app_home("config"));
pub static LOGOS_DIR: LazyLock<String> = LazyLock::new(|| app_home("logos"));
pub static LOG_DIR: LazyLock<String> = LazyLock::new(|| app_home("logs"));
pub static LIB_VK_DIR: LazyLock<String> = LazyLock::new(|| app_home("lib/vk/icd.d/"));
pub static LIB_OCL_DIR: LazyLock<String> = LazyLock::new(|| app_home("lib/ocl/icd.d/"));
pub static USER_PLUGIN_DIR: LazyLock<String> = LazyLock::new(|| app_home("plugin"));
pub static AUTOSTART_FILE: LazyLock<String> =
    LazyLock::new(|| format!("{}/.config/autostart/{APP_NAME}.desktop", *HOME_DIR));
pub static STEAM_USERDATA_PATH: LazyLock<String> =
    LazyLock::new(|| format!("{}/.steam/steam/userdata", *HOME_DIR));
pub static PLUGINS_FOLDER: LazyLock<String> = LazyLock::new(|| format!("{}/homebrew/plugins", *HOME_DIR));
pub static RCCDC_PATH: LazyLock<String> =
    LazyLock::new(|| format!("{}/homebrew/plugins/RCCDeckyCompanion", *HOME_DIR));
pub static RCCDC_PACKAGE_FILE: LazyLock<String> =
    LazyLock::new(|| format!("{}/homebrew/plugins/RCCDeckyCompanion/package.json", *HOME_DIR));
pub static DECKY_SERVICE_PATH: LazyLock<String> =
    LazyLock::new(|| format!("{}/homebrew/services/PluginLoader", *HOME_DIR));
