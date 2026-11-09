//! Runtime hardware feature detection.
//!
//! Features are detected once at startup, so the same binary works on any machine.

use std::{path::Path, process::Command, sync::LazyLock};

use log::info;
use serde::Serialize;

use crate::utils::logger::tabbed;

const ARMOURY: &str = "/sys/class/firmware-attributes/asus-armoury/attributes";

#[derive(Debug, Clone, Serialize)]
pub struct GpuInfo {
    pub brand: String,
    pub name: String,
    pub env: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct Features {
    pub acpi_profile: Option<String>,
    pub bat_limit: Option<String>,
    pub bat_status: Option<String>,
    /// (file, on value, off value)
    pub boost_control: Option<(String, String, String)>,
    pub boot_sound: Option<String>,
    pub fan_control: bool,
    pub intel_rapl_uj: Option<String>,
    pub ntsync: bool,
    pub nv_boost: Option<String>,
    pub nv_thermal: Option<String>,
    pub panel_od: Option<String>,
    pub ppt_pl1_spl: Option<String>,
    pub ppt_pl2_sppt: Option<String>,
    pub ppt_pl3_fppt: Option<String>,
    pub scaling_governor: Option<String>,
    pub cpu_name: String,
    pub gpu: Option<GpuInfo>,
}

fn first_glob(pattern: &str) -> Option<String> {
    glob::glob(pattern).ok()?.flatten().next().map(|p| p.display().to_string())
}

fn is_file(path: &str) -> Option<String> {
    Path::new(path).is_file().then(|| path.to_string())
}

fn is_dir(path: &str) -> Option<String> {
    Path::new(path).is_dir().then(|| path.to_string())
}

fn run(cmd: &str) -> Option<String> {
    Command::new("bash")
        .args(["-c", cmd])
        .env("LANG", "C")
        .output()
        .ok()
        .filter(|o| o.status.success())
        .map(|o| String::from_utf8_lossy(&o.stdout).to_string())
}

fn detect_cpu() -> String {
    let name = std::fs::read_to_string("/proc/cpuinfo")
        .ok()
        .and_then(|c| {
            c.lines()
                .find(|l| l.starts_with("model name"))
                .and_then(|l| l.split_once(':').map(|(_, v)| v.trim().to_string()))
        })
        .unwrap_or_default();
    regex::Regex::new(r"\s*\([^)]*\)").unwrap().replace_all(&name, "").to_string()
}

/// First AMD/NVIDIA device reported by switcherooctl.
fn detect_gpu() -> Option<GpuInfo> {
    let out = run("switcherooctl")?;
    let mut devices: Vec<(String, Vec<String>)> = vec![];
    for line in out.lines().map(str::trim) {
        if line.starts_with("Device:") {
            devices.push((String::new(), vec![]));
        } else if let Some(name) = line.strip_prefix("Name:") {
            if let Some(d) = devices.last_mut() {
                d.0 = name.trim().to_string();
            }
        } else if let Some(env) = line.strip_prefix("Environment:")
            && let Some(d) = devices.last_mut()
        {
            d.1.extend(env.split_whitespace().map(|s| s.to_string()));
        }
    }
    devices.into_iter().find_map(|(name, env)| {
        if name.contains("Advanced Micro Devices") || name.to_uppercase().contains("NVIDIA") {
            let name = name.replace("Advanced Micro Devices, Inc.", "AMD");
            let brand = name.split(' ').next().unwrap_or_default().to_lowercase();
            Some(GpuInfo { brand, name, env: env.join(" ") })
        } else {
            None
        }
    })
}

fn detect() -> Features {
    let mut f = Features {
        cpu_name: detect_cpu(),
        ..Default::default()
    };
    f.acpi_profile = is_file("/sys/firmware/acpi/platform_profile");
    f.bat_limit = first_glob("/sys/class/power_supply/BAT[0-9]*/charge_control_end_threshold");
    f.bat_status = first_glob("/sys/class/power_supply/BAT[0-9]*/status");
    for (file, on, off) in [
        ("/sys/devices/system/cpu/intel_pstate/no_turbo", "0", "1"),
        ("/sys/devices/system/cpu/cpufreq/boost", "1", "0"),
    ] {
        if Path::new(file).is_file() {
            f.boost_control = Some((file.into(), on.into(), off.into()));
            break;
        }
    }
    f.boot_sound = is_dir(&format!("{ARMOURY}/boot_sound"));
    f.fan_control = run("asusctl fan-curve --get-enabled")
        .map(|o| o.lines().count() > 0)
        .unwrap_or(false);
    if f.cpu_name.to_lowercase().contains("intel") {
        f.intel_rapl_uj = first_glob("/sys/class/powercap/intel-rapl:[0-9]/energy_uj");
    }
    f.ntsync = Path::new("/dev/ntsync").exists();
    f.nv_boost = is_dir(&format!("{ARMOURY}/nv_dynamic_boost"));
    f.nv_thermal = is_dir(&format!("{ARMOURY}/nv_temp_target"));
    f.panel_od = is_dir(&format!("{ARMOURY}/panel_overdrive"));
    f.ppt_pl1_spl = is_dir(&format!("{ARMOURY}/ppt_pl1_spl"));
    if f.ppt_pl1_spl.is_some() {
        f.ppt_pl2_sppt = is_dir(&format!("{ARMOURY}/ppt_pl2_sppt"));
        if f.ppt_pl2_sppt.is_some() {
            f.ppt_pl3_fppt = is_dir(&format!("{ARMOURY}/ppt_pl3_fppt"));
        }
    }
    let gov = "/sys/devices/system/cpu/cpu*/cpufreq/scaling_governor";
    f.scaling_governor = first_glob(gov).map(|_| gov.to_string());
    f.gpu = detect_gpu();
    f
}

pub static FEATURES: LazyLock<Features> = LazyLock::new(detect);

pub fn features() -> &'static Features {
    &FEATURES
}

/// Logs the table of detected features.
pub fn print_feature_table() {
    let f = features();
    let rows = [
        ("Dev mode", *crate::utils::constants::DEV_MODE),
        ("ACPI profiles", f.acpi_profile.is_some()),
        ("Battery limit", f.bat_limit.is_some()),
        ("Battery status", f.bat_status.is_some()),
        ("Boost control", f.boost_control.is_some()),
        ("Boot sound", f.boot_sound.is_some()),
        ("Fan control", f.fan_control),
        ("Intel rapl UJ", f.intel_rapl_uj.is_some()),
        ("NTSync", f.ntsync),
        ("Nvidia boost", f.nv_boost.is_some()),
        ("Nvidia thermal", f.nv_thermal.is_some()),
        ("Panel overdrive", f.panel_od.is_some()),
        ("Scaling governor", f.scaling_governor.is_some()),
        ("TDP PL1 SPD", f.ppt_pl1_spl.is_some()),
        ("TDP PL2 SPPT", f.ppt_pl2_sppt.is_some()),
        ("TDP PL3 FPPT", f.ppt_pl3_fppt.is_some()),
    ];
    tabbed(|| {
        info!(target: "Main", "┌──────────────────┐");
        info!(target: "Main", "│     Features     │");
        info!(target: "Main", "├──────────────────┤");
        for (name, enabled) in rows {
            if enabled {
                info!(target: "Main", "│ {name:<16} │");
            }
        }
        info!(target: "Main", "└──────────────────┘");
    });
}
