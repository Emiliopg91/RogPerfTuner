//! System tray menu.

use std::sync::Mutex;

use tauri::{
    AppHandle, Wry,
    menu::{CheckMenuItem, IsMenuItem, Menu, MenuItem, PredefinedMenuItem, Submenu},
    tray::{MouseButton, MouseButtonState, TrayIconBuilder, TrayIconEvent},
};

use crate::{
    features::features,
    models::{
        hardware::{BatteryThreshold, RgbBrightness},
        performance::PerformanceProfile,
    },
    services::{hardware, openrgb, performance, steam},
    utils::{capitalize, constants, translator::translate},
};

/// Check items grouped by the value they represent, to refresh their state on events.
#[derive(Default)]
struct TrayItems {
    profiles: Vec<(String, CheckMenuItem<Wry>)>,
    schedulers: Vec<(String, CheckMenuItem<Wry>)>,
    ssd_schedulers: Vec<(String, CheckMenuItem<Wry>)>,
    effects: Vec<(String, CheckMenuItem<Wry>)>,
    brightness: Vec<(String, CheckMenuItem<Wry>)>,
    charge: Vec<(String, CheckMenuItem<Wry>)>,
    boot_sound: Vec<(String, CheckMenuItem<Wry>)>,
}

static ITEMS: Mutex<Option<TrayItems>> = Mutex::new(None);

fn check_group(
    app: &AppHandle,
    prefix: &str,
    options: &[(String, String)],
) -> tauri::Result<Vec<(String, CheckMenuItem<Wry>)>> {
    options
        .iter()
        .map(|(value, label)| {
            CheckMenuItem::with_id(app, format!("{prefix}:{value}"), label, true, false, None::<&str>)
                .map(|item| (value.clone(), item))
        })
        .collect()
}

fn submenu(
    app: &AppHandle,
    title: &str,
    items: &[(String, CheckMenuItem<Wry>)],
) -> tauri::Result<Submenu<Wry>> {
    let refs: Vec<&dyn IsMenuItem<Wry>> = items.iter().map(|(_, i)| i as &dyn IsMenuItem<Wry>).collect();
    Submenu::with_items(app, format!("    {title}"), true, &refs)
}

fn title(app: &AppHandle, text: &str) -> tauri::Result<MenuItem<Wry>> {
    MenuItem::new(app, text, false, None::<&str>)
}

pub fn build(app: &AppHandle) -> tauri::Result<()> {
    let perf = performance::get();
    let rgb = openrgb::get();
    let hw = hardware::get();
    let mut items = TrayItems::default();
    let mut entries: Vec<Box<dyn IsMenuItem<Wry>>> = vec![];
    let sep = || PredefinedMenuItem::separator(app);

    // Hardware
    if hw.charge_threshold().is_some() || hw.boot_sound().is_some() {
        entries.push(Box::new(title(app, &translate("hardware"))?));
        if hw.charge_threshold().is_some() {
            let opts: Vec<_> = BatteryThreshold::ALL
                .iter()
                .map(|t| (t.value().to_string(), format!("{}%", t.value())))
                .collect();
            items.charge = check_group(app, "charge", &opts)?;
            entries.push(Box::new(submenu(app, &translate("charge.threshold"), &items.charge)?));
        }
        if hw.boot_sound().is_some() {
            let opts = vec![
                ("true".to_string(), translate("enabled")),
                ("false".to_string(), translate("disabled")),
            ];
            items.boot_sound = check_group(app, "boot", &opts)?;
            entries.push(Box::new(submenu(app, &translate("boot.sound"), &items.boot_sound)?));
        }
        entries.push(Box::new(sep()?));
    }

    // Aura
    entries.push(Box::new(title(app, "Aura")?));
    let opts: Vec<_> = rgb.available_effects().into_iter().map(|e| (e.clone(), e)).collect();
    items.effects = check_group(app, "effect", &opts)?;
    entries.push(Box::new(submenu(app, &translate("effect"), &items.effects)?));
    let opts: Vec<_> = RgbBrightness::ALL
        .iter()
        .map(|b| (b.value().to_string(), translate(&format!("label.brightness.{}", b.name()))))
        .collect();
    items.brightness = check_group(app, "brightness", &opts)?;
    entries.push(Box::new(submenu(app, &translate("brightness"), &items.brightness)?));
    entries.push(Box::new(sep()?));

    // Performance
    entries.push(Box::new(title(app, &translate("performance"))?));
    let opts: Vec<_> = PerformanceProfile::ALL
        .iter()
        .map(|p| (p.as_str().to_string(), translate(&format!("label.profile.{}", p.name()))))
        .collect();
    items.profiles = check_group(app, "profile", &opts)?;
    entries.push(Box::new(submenu(app, &translate("profile"), &items.profiles)?));
    let opts: Vec<_> = perf.available_schedulers().into_iter().map(|s| (s.clone(), s)).collect();
    items.schedulers = check_group(app, "sched", &opts)?;
    entries.push(Box::new(submenu(app, &translate("scheduler"), &items.schedulers)?));
    let ssd = perf.available_ssd_schedulers();
    if !ssd.is_empty() {
        let opts: Vec<_> = ssd
            .into_iter()
            .map(|s| {
                let label = capitalize(&if s == "none" { translate("default") } else { s.clone() });
                (s, label)
            })
            .collect();
        items.ssd_schedulers = check_group(app, "ssd", &opts)?;
        entries.push(Box::new(submenu(app, &translate("ssd.scheduler"), &items.ssd_schedulers)?));
    }
    if features().fan_control {
        let edit = MenuItem::with_id(app, "fans", format!("{}...", translate("edit.curve")), true, None::<&str>)?;
        entries.push(Box::new(Submenu::with_items(app, format!("    {}", translate("fan.curves")), true, &[&edit])?));
    }
    let games = MenuItem::with_id(app, "games", format!("{}...", translate("label.game.configure")), true, None::<&str>)?;
    entries.push(Box::new(Submenu::with_items(app, format!("    {}", translate("games")), true, &[&games])?));
    entries.push(Box::new(sep()?));

    // Logs
    let log_main = MenuItem::with_id(app, "log:main", translate("open.log"), true, None::<&str>)?;
    let log_runner = MenuItem::with_id(app, "log:runner", translate("open.log.runner"), true, None::<&str>)?;
    let log_orgb = MenuItem::with_id(app, "log:orgb", translate("open.log.openrgb"), true, None::<&str>)?;
    entries.push(Box::new(Submenu::with_items(app, translate("logs"), true, &[&log_main, &log_runner, &log_orgb])?));
    entries.push(Box::new(sep()?));

    if *constants::DEV_MODE {
        let open_cfg = MenuItem::with_id(app, "settings:open", "Open settings", true, None::<&str>)?;
        let reload_cfg = MenuItem::with_id(app, "settings:reload", "Reload settings", true, None::<&str>)?;
        entries.push(Box::new(Submenu::with_items(app, "Settings", true, &[&open_cfg, &reload_cfg])?));
        entries.push(Box::new(sep()?));
    }

    entries.push(Box::new(MenuItem::with_id(app, "open", translate("open.ui"), true, None::<&str>)?));
    entries.push(Box::new(sep()?));
    entries.push(Box::new(MenuItem::with_id(app, "quit", translate("close"), true, None::<&str>)?));

    let refs: Vec<&dyn IsMenuItem<Wry>> = entries.iter().map(|e| e.as_ref()).collect();
    let menu = Menu::with_items(app, &refs)?;

    *ITEMS.lock().unwrap() = Some(items);
    refresh();

    TrayIconBuilder::with_id("main")
        .icon(app.default_window_icon().cloned().ok_or(tauri::Error::InvalidIcon(std::io::Error::other("no icon")))?)
        .tooltip(format!("{} v{}", constants::APP_NAME, constants::APP_VERSION))
        .menu(&menu)
        .show_menu_on_left_click(false)
        .on_menu_event(|app, event| on_menu_event(app, event.id.as_ref()))
        .on_tray_icon_event(|tray, event| {
            if let TrayIconEvent::Click {
                button: MouseButton::Left,
                button_state: MouseButtonState::Up,
                ..
            } = event
            {
                crate::show_main_window(tray.app_handle(), None);
            }
        })
        .build(app)?;
    Ok(())
}

fn on_menu_event(app: &AppHandle, id: &str) {
    let app = app.clone();
    let id = id.to_string();
    // Service calls can block, keep them off the main thread
    std::thread::spawn(move || {
        let (kind, value) = id.split_once(':').unwrap_or((id.as_str(), ""));
        match kind {
            "profile" => {
                if let Some(p) = PerformanceProfile::from_str(value)
                    && steam::get().running_games().is_empty()
                {
                    performance::get().set_performance_profile(p, false, false, true);
                }
            }
            "sched" => {
                if steam::get().running_games().is_empty() {
                    performance::get().set_scheduler(value, false);
                }
            }
            "ssd" => performance::get().set_ssd_scheduler(value, false),
            "effect" => openrgb::get().set_effect(value, false),
            "brightness" => {
                if let Some(b) = value.parse::<u32>().ok().and_then(|v| RgbBrightness::try_from(v).ok()) {
                    openrgb::get().set_brightness(b);
                }
            }
            "charge" => {
                if let Some(t) = value.parse::<u32>().ok().and_then(|v| BatteryThreshold::try_from(v).ok()) {
                    let _ = hardware::get().set_charge_threshold(t);
                }
            }
            "boot" => {
                let _ = hardware::get().set_boot_sound(value == "true");
            }
            "fans" => crate::show_main_window(&app, Some("fans".into())),
            "games" => crate::show_main_window(&app, Some("games".into())),
            "open" => crate::show_main_window(&app, None),
            "log" => {
                let file = match value {
                    "runner" => constants::LOG_RUNNER_FILE_NAME,
                    "orgb" => constants::LOG_ORGB_FILE_NAME,
                    _ => constants::LOG_FILE_NAME,
                };
                let _ = std::process::Command::new("xdg-open")
                    .arg(format!("{}/{file}.log", *constants::LOG_DIR))
                    .spawn();
            }
            "settings" => {
                if value == "reload" {
                    crate::services::configuration::reload();
                } else {
                    let _ = std::process::Command::new("xdg-open")
                        .arg(format!("{}/{}", *constants::CONFIG_DIR, constants::CONFIG_FILE))
                        .spawn();
                }
            }
            "quit" => crate::quit(&app),
            _ => {}
        }
        refresh();
    });
}

fn set_group(group: &[(String, CheckMenuItem<Wry>)], selected: &str, enabled: bool) {
    for (value, item) in group {
        let _ = item.set_checked(value == selected);
        let _ = item.set_enabled(enabled);
    }
}

/// Syncs the check marks with the current service state.
pub fn refresh() {
    let guard = ITEMS.lock().unwrap();
    let Some(items) = guard.as_ref() else {
        return;
    };
    let perf = performance::get();
    let rgb = openrgb::get();
    let hw = hardware::get();
    let no_games = steam::get().running_games().is_empty();
    set_group(&items.profiles, perf.performance_profile().as_str(), no_games && !hw.on_battery());
    set_group(&items.schedulers, &perf.current_scheduler(), no_games);
    set_group(&items.ssd_schedulers, &perf.current_ssd_scheduler().unwrap_or_default(), true);
    set_group(&items.effects, &rgb.current_effect(), true);
    set_group(&items.brightness, &rgb.current_brightness().value().to_string(), true);
    if let Some(t) = hw.charge_threshold() {
        set_group(&items.charge, &t.value().to_string(), true);
    }
    if let Some(b) = hw.boot_sound() {
        set_group(&items.boot_sound, &b.to_string(), true);
    }
}
