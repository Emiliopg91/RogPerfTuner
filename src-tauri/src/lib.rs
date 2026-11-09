mod cli;
mod clients;
mod commands;
mod dto;
mod events;
mod features;
mod models;
mod servers;
mod services;
mod tray;
mod utils;

use std::{
    sync::{
        atomic::{AtomicBool, Ordering},
        Mutex, OnceLock,
        mpsc::{Sender, channel},
    },
    thread,
    time::Instant,
};

use log::{error, info};
use tauri::{AppHandle, Manager, RunEvent, WindowEvent};

use crate::{
    clients::file::proc_modules_client::is_module_loaded,
    dto::{app::InitStatus, games::OpenGameConfig},
    services::{application, configuration, hardware, openrgb, performance, steam},
    utils::{
        constants, elapsed_secs,
        event_bus::{self, Event},
        logger::{self, tabbed},
        shell::Shell,
    },
};

const LOG: &str = "Main";

static APP: OnceLock<AppHandle> = OnceLock::new();
static STATUS: Mutex<InitStatus> = Mutex::new(InitStatus::Loading);
static PASSWORD_TX: Mutex<Option<Sender<String>>> = Mutex::new(None);

pub use cli::handle as handle_cli;

pub(crate) fn status() -> InitStatus {
    *STATUS.lock().unwrap()
}

fn set_status(status: InitStatus) {
    *STATUS.lock().unwrap() = status;
    if let Some(app) = APP.get() {
        events::emit_status_changed(app, status);
    }
}

/// Hands the password typed in the GUI to the waiting initialization thread.
pub(crate) fn provide_password(password: String) {
    if let Some(tx) = PASSWORD_TX.lock().unwrap().take() {
        let _ = tx.send(password);
    }
}

pub(crate) fn show_main_window(app: &AppHandle, view: Option<String>) {
    if let Some(window) = app.get_webview_window("main") {
        let _ = window.show();
        let _ = window.unminimize();
        let _ = window.set_focus();
    }
    if let Some(view) = view {
        events::emit_show_view(app, view);
    }
}

pub(crate) fn hide_main_window(app: &AppHandle) {
    if let Some(window) = app.get_webview_window("main") {
        let _ = window.hide();
    }
}

/// Graceful shutdown: restores the hardware state and exits.
pub(crate) fn quit(app: &AppHandle) {
    if status() == InitStatus::Ready {
        application::shutdown();
    }
    app.exit(0);
}

static REFRESH_PENDING: AtomicBool = AtomicBool::new(false);

/// Refreshes the frontend state and the tray once per burst of events: one action usually emits several
/// (e.g. effect + color), and every refresh reloads the whole state.
fn schedule_state_refresh(app: &AppHandle) {
    if REFRESH_PENDING.swap(true, Ordering::SeqCst) {
        return;
    }
    let app = app.clone();
    thread::spawn(move || {
        thread::sleep(std::time::Duration::from_millis(50));
        REFRESH_PENDING.store(false, Ordering::SeqCst);
        events::emit_state_changed(&app);
        tray::refresh();
    });
}

/// Forwards bus events to the frontend and the tray.
fn bridge_events(app: AppHandle) {
    event_bus::subscribe(move |e| match e {
        Event::PerformanceProfile(_)
        | Event::Scheduler(_)
        | Event::SsdScheduler(_)
        | Event::RgbBrightness(_)
        | Event::RgbEffect(_)
        | Event::RgbColor(_)
        | Event::ChargeThreshold(_)
        | Event::BootSound(_)
        | Event::OnBattery(_) => schedule_state_refresh(&app),
        Event::GamesChanged(_) => {
            schedule_state_refresh(&app);
            events::emit_games_changed(&app);
        }
        Event::OpenGameConfig { gid, first_run } => {
            show_main_window(&app, None);
            events::emit_open_game_config(
                &app,
                OpenGameConfig {
                    id: *gid,
                    first_run: *first_run,
                },
            );
            events::emit_games_changed(&app);
        }
        Event::AskDeckyInstall => {
            show_main_window(&app, None);
            events::emit_ask_decky_install(&app);
        }
        Event::ShowGui => show_main_window(&app, None),
        Event::ApplicationStop => {
            info!(target: LOG, "Application finished");
            std::process::exit(0);
        }
        _ => {}
    });
}

/// Initialization sequence, run off the main thread.
fn initialize(app: AppHandle, exec_path: String, t0: Instant) {
    let mut password = configuration::password();
    if password.is_empty() {
        let (tx, rx) = channel();
        *PASSWORD_TX.lock().unwrap() = Some(tx);
        set_status(InitStatus::Password);
        show_main_window(&app, None);
        password = rx.recv().unwrap_or_default();
        set_status(InitStatus::Loading);
    }
    // A single elevated session (`Shell::init(Some(..))` would also open one)
    Shell::get().set_password(&password);

    if !is_module_loaded("asus_armoury") {
        log::error!(target: LOG, "Missing 'asus_armoury' kernel module");
        set_status(InitStatus::MissingModule);
        show_main_window(&app, None);
        return;
    }

    application::init(&exec_path);
    // Dependency order: OpenRGB <- Hardware <- Performance <- Steam
    openrgb::get().start();
    hardware::get().start();
    performance::get().start();
    steam::get().start();
    servers::socket_server::start();

    let handle = app.clone();
    if let Err(e) = app.run_on_main_thread(move || {
        if let Err(e) = tray::build(&handle) {
            error!(target: LOG, "Couldn't create tray icon: {e}");
        }
    }) {
        error!(target: LOG, "Couldn't create tray icon: {e}");
    }

    logger::rem_tab();
    info!(target: LOG, "Application ready after {} seconds", elapsed_secs(t0));
    set_status(InitStatus::Ready);
    events::emit_state_changed(&app);

    if !application::is_start_minimized() {
        show_main_window(&app, None);
    }
}

/// Turns SIGTERM/SIGINT/SIGHUP into a graceful shutdown. Must run before any other thread is spawned,
/// so every thread inherits the blocked signal mask.
fn listen_for_signals() {
    unsafe {
        let mut set: libc::sigset_t = std::mem::zeroed();
        libc::sigemptyset(&mut set);
        for sig in [libc::SIGTERM, libc::SIGINT, libc::SIGHUP] {
            libc::sigaddset(&mut set, sig);
        }
        libc::pthread_sigmask(libc::SIG_BLOCK, &set, std::ptr::null_mut());
        let set_copy = set;
        thread::spawn(move || {
            let mut sig: libc::c_int = 0;
            if libc::sigwait(&set_copy, &mut sig) == 0 {
                info!(target: LOG, "Received signal {sig}, shutting down...");
                if status() == InitStatus::Ready {
                    application::shutdown();
                }
                std::process::exit(0);
            }
        });
    }
}

fn executable_path(argv0: &str) -> String {
    if !argv0.contains('/') {
        return argv0.to_string();
    }
    std::fs::canonicalize(argv0)
        .map(|p| p.display().to_string())
        .unwrap_or_else(|_| argv0.to_string())
}

/// Starts the GUI application (`startGui()`).
pub fn run(argv0: &str) {
    let t0 = Instant::now();
    listen_for_signals();
    let exec_path = executable_path(argv0);

    utils::single_instance::acquire_single_instance(&constants::LOCK_FILE);
    logger::initialize(Some(constants::LOG_FILE_NAME), Some(&constants::LOG_DIR));

    let title = format!("Starting {}", constants::APP_NAME);
    info!(target: LOG, "###################################################");
    info!(target: LOG, "#{title:^49}#");
    info!(target: LOG, "###################################################");
    info!(target: LOG, "Version {}", constants::APP_VERSION);
    if *constants::DEV_MODE {
        info!(target: LOG, "Running application with PID {}", *constants::PID);
        info!(target: LOG, "Executable path: {exec_path}");
        info!(target: LOG, "Assets directory: {}", *constants::ASSETS_DIR);
    }
    features::print_feature_table();
    info!(target: LOG, "Starting initialization");
    logger::add_tab();

    tauri::Builder::default()
        .setup(move |app| {
            let handle = app.handle().clone();
            let _ = APP.set(handle.clone());
            bridge_events(handle.clone());

            if let Some(window) = app.get_webview_window("main") {
                let _ = window.set_title(&format!("{} | {}", constants::APP_NAME, constants::APP_VERSION));
                let h = handle.clone();
                window.on_window_event(move |event| {
                    if let WindowEvent::CloseRequested { api, .. } = event {
                        // Closing only hides the window, the app lives in the tray
                        if status() == InitStatus::Ready {
                            api.prevent_close();
                            hide_main_window(&h);
                        }
                    }
                });
            }

            tabbed(|| info!(target: LOG, "Creating Tauri application"));
            thread::spawn(move || initialize(handle, exec_path, t0));
            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            commands::get_bootstrap,
            commands::submit_password,
            commands::get_state,
            commands::set_profile,
            commands::set_scheduler,
            commands::set_ssd_scheduler,
            commands::set_effect,
            commands::set_brightness,
            commands::set_color,
            commands::set_charge_limit,
            commands::set_boot_sound,
            commands::set_autostart,
            commands::set_start_minimized,
            commands::get_games,
            commands::get_game_config,
            commands::get_game_suggestion,
            commands::save_game_config,
            commands::launch_game,
            commands::answer_decky_install,
            commands::get_fan_curves,
            commands::save_fan_curves,
            commands::hide_window,
            commands::quit,
        ])
        .build(tauri::generate_context!())
        .unwrap_or_else(|e| {
            eprintln!("Error while running tauri application {e}");
            std::process::exit(1)
        })
        .run(|app, event| match event {
            // Keep running in the tray when the last window is closed
            RunEvent::ExitRequested { api, code, .. } if code.is_none() && status() == InitStatus::Ready => {
                api.prevent_exit();
                hide_main_window(app);
            }
            RunEvent::Exit => info!(target: LOG, "Stopped {} v{}", constants::APP_NAME, constants::APP_VERSION),
            _ => {}
        });
}
