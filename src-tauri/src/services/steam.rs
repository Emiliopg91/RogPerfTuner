//! Steam integration through the RCCDeckyCompanion plugin: per game configuration,
//! performance mode while gaming and launch wrapper configuration.

use std::{
    collections::BTreeMap,
    sync::{LazyLock, Mutex},
    thread,
};

use log::{debug, error, info, warn};

use crate::{
    clients::{
        file::intel_rapl_uj_client::enable_intel_rapl_read,
        unix_socket::steam_client::{self, STEAM},
    },
    models::{
        performance::PerformanceProfile,
        settings::{ComputerType, GameEntry, MangoHudLevel, WineSyncOption},
        steam::{SemanticVersion, SteamGameConfig},
    },
    services::{configuration, hardware, openrgb, performance},
    utils::{
        self, constants,
        event_bus::{self, Event},
        logger::tabbed,
        shell::Shell,
        sleep_ms,
    },
};

const LOG: &str = "SteamService";

pub struct SteamService {
    running_games: Mutex<BTreeMap<u32, GameEntry>>,
    which_mangohud: Option<String>,
    which_systemd_inhibit: Option<String>,
}

static SERVICE: LazyLock<SteamService> = LazyLock::new(|| {
    info!(target: LOG, "Initializing SteamService");
    tabbed(|| {
        let shell = Shell::get();
        let which_mangohud = shell.which("mangohud");
        if which_mangohud.is_some() {
            info!(target: LOG, "Metric level service available");
            tabbed(|| {
                if crate::features::features().intel_rapl_uj.is_some() {
                    info!(target: LOG, "Enabling Intel CPU Wattage report...");
                    enable_intel_rapl_read();
                }
            });
        }
        let _ = std::fs::create_dir_all(constants::LOGOS_DIR.as_str());
        SteamService {
            running_games: Mutex::new(BTreeMap::new()),
            which_mangohud,
            which_systemd_inhibit: shell.which("systemd-inhibit"),
        }
    })
});

pub fn get() -> &'static SteamService {
    &SERVICE
}

impl SteamService {
    pub fn start(&'static self) {
        event_bus::subscribe(|e| match e {
            Event::SteamConnected => get().on_connect(false),
            Event::SteamDisconnected => get().on_disconnect(),
            Event::SteamGameLaunch { id, name, pid } => {
                let (id, name, pid) = (*id, name.clone(), *pid);
                thread::spawn(move || get().on_game_launch(id, &name, pid));
            }
            Event::SteamGameStop { id, name } => {
                let (id, name) = (*id, name.clone());
                thread::spawn(move || get().on_game_stop(id, &name));
            }
            _ => {}
        });
        if STEAM.connected() {
            self.on_connect(true);
        }
        self.install_rccdc(false);
    }

    pub fn metrics_enabled(&self) -> bool {
        self.which_mangohud.is_some()
    }

    pub fn running_games(&self) -> Vec<u32> {
        self.running_games.lock().unwrap().keys().copied().collect()
    }

    pub fn is_running(&self, app_id: u32) -> bool {
        self.running_games.lock().unwrap().contains_key(&app_id)
    }

    pub fn games(&self) -> BTreeMap<u32, GameEntry> {
        configuration::read(|c| c.games.clone())
    }

    fn on_connect(&self, on_boot: bool) {
        info!(target: LOG, "Connected to Steam");
        tabbed(|| {
            if !on_boot {
                self.set_profile_for_games(true);
            }
            event_bus::emit(Event::GamesChanged(self.running_games.lock().unwrap().len()));
        });
    }

    fn on_disconnect(&self) {
        info!(target: LOG, "Disconnected from Steam");
        tabbed(|| {
            let had_games = !self.running_games.lock().unwrap().is_empty();
            if had_games {
                performance::get().restore();
                openrgb::get().restore_aura();
                self.running_games.lock().unwrap().clear();
                event_bus::emit(Event::GamesChanged(0));
            }
        });
    }

    fn on_game_launch(&self, gid: u32, name: &str, pid: i32) {
        info!(target: LOG, "Launched {name} ({gid}) with PID {pid}");
        tabbed(|| {
            let entry = configuration::read(|c| c.games.get(&gid).cloned());
            match entry {
                None => {
                    info!(target: LOG, "Game not configured");
                    let proton = tabbed(|| {
                        let cmdline = utils::process::cmdline(pid);
                        debug!(target: LOG, "CMD line: {}", cmdline.join(" "));
                        let proton = cmdline.iter().rev().any(|a| a.to_lowercase().ends_with(".exe"));

                        info!(target: LOG, "Stopping process...");
                        let mut signaled = Default::default();
                        loop {
                            let new = utils::process::send_signal_to_hierarchy(pid, libc::SIGSTOP);
                            debug!(target: LOG, "Stopped {} processes", new.len());
                            sleep_ms(100);
                            if new == signaled {
                                break;
                            }
                            signaled = new;
                        }
                        debug!(target: LOG, "Killed {} processes", utils::process::send_signal_to_hierarchy(pid, libc::SIGKILL).len());
                        proton
                    });
                    let name = name.to_string();
                    thread::spawn(move || get().on_first_game_run(gid, &name, proton));
                }
                Some(entry) => {
                    let first = {
                        let mut running = self.running_games.lock().unwrap();
                        running.insert(gid, entry).is_none()
                    };
                    if first {
                        performance::get().renice(pid);
                        self.set_profile_for_games(false);
                        event_bus::emit(Event::GamesChanged(self.running_games.lock().unwrap().len()));
                    }
                }
            }
        });
    }

    fn on_game_stop(&self, gid: u32, name: &str) {
        let Some(stopped) = self.running_games.lock().unwrap().remove(&gid) else {
            return;
        };
        info!(target: LOG, "Stopped '{name}' ({gid})");
        tabbed(|| {
            if stopped.scheduler.is_some() {
                let sched = self.running_games.lock().unwrap().values().find_map(|g| g.scheduler.clone());
                let perf = performance::get();
                let target = sched.clone().unwrap_or_else(|| perf.current_scheduler());
                perf.set_scheduler(&target, sched.is_some());
            }
            self.set_profile_for_games(false);
            event_bus::emit(Event::GamesChanged(self.running_games.lock().unwrap().len()));
        });
    }

    fn set_profile_for_games(&self, on_connect: bool) {
        let running: Vec<GameEntry> = self.running_games.lock().unwrap().values().cloned().collect();
        let perf = performance::get();
        if !running.is_empty() {
            hardware::get().set_panel_overdrive(true);
            openrgb::get().set_effect("Gaming", true);
            perf.set_performance_profile(PerformanceProfile::Performance, true, true, true);
            let current = perf.current_scheduler();
            let sched = running
                .iter()
                .find_map(|g| g.scheduler.clone().filter(|s| *s != current))
                .or_else(|| configuration::read(|c| c.platform.performance.scheduler.clone()))
                .unwrap_or_else(|| perf.default_scheduler());
            perf.set_scheduler(&sched, true);
        } else if !on_connect {
            hardware::get().set_panel_overdrive(false);
            openrgb::get().restore_aura();
            perf.restore();
        }
    }

    fn on_first_game_run(&self, gid: u32, name: &str, proton: bool) {
        info!(target: LOG, "Configuring game");
        tabbed(|| {
            let details = steam_client::apps_details(&[gid])
                .map_err(|e| error!(target: LOG, "Error getting app details: {e}"))
                .ok()
                .and_then(|d| d.into_iter().next());
            let (launch_opts, is_shortcut, icon_hash) = details
                .as_ref()
                .map(|d| (d.launch_opts.clone(), d.is_shortcut, d.icon_hash.clone()))
                .unwrap_or_else(|| ("%command%".into(), false, String::new()));

            let (mut env, mut args) = (String::new(), launch_opts.trim().to_string());
            if let Some(pos) = launch_opts.find("%command%") {
                env = launch_opts[..pos].trim().to_string();
                args = launch_opts[pos + "%command%".len()..].trim().to_string();
            }
            let mut pure_env = vec![];
            let mut wrappers = vec![];
            for tok in env.split_whitespace() {
                if tok.contains('=') {
                    pure_env.push(tok);
                } else if tok != constants::STEAM_WRAPPER_PATH.as_str() {
                    wrappers.push(tok);
                }
            }

            let encoded = ((gid as u64) << 32) | 0x0200_0000;
            let entry = GameEntry {
                name: name.to_string(),
                args: Some(args),
                env: Some(pure_env.join(" ")),
                overlay_id: is_shortcut.then(|| encoded.to_string()),
                proton,
                wrappers: Some(wrappers.join(" ")),
                ..Default::default()
            };
            configuration::update(|c| c.games.insert(gid, entry));

            let users = utils::files::list_dir(&constants::STEAM_USERDATA_PATH);
            for suffix in ["logo", "hero"] {
                for user in &users {
                    let src = format!("{}/{user}/config/grid/{gid}_{suffix}.png", *constants::STEAM_USERDATA_PATH);
                    if utils::files::exists(&src) {
                        let _ = utils::files::copy_path(&src, &format!("{}/{gid}_{suffix}.png", *constants::LOGOS_DIR));
                        break;
                    }
                }
            }
            if !icon_hash.is_empty() && self.icon(gid).is_none() {
                let _ = utils::net::download(
                    &format!("https://cdn.cloudflare.steamstatic.com/steamcommunity/public/images/apps/{gid}/{icon_hash}.jpg"),
                    &format!("{}/{gid}_logo.jpg", *constants::LOGOS_DIR),
                );
            }
            if self.banner(gid).is_none() {
                let _ = utils::net::download(
                    &format!("https://cdn.cloudflare.steamstatic.com/steam/apps/{gid}/header.jpg"),
                    &format!("{}/{gid}_hero.jpg", *constants::LOGOS_DIR),
                );
            }

            if let Err(e) = steam_client::set_launch_options(gid, &format!("{} %command%", *constants::STEAM_WRAPPER_PATH)) {
                error!(target: LOG, "Error setting launch options: {e}");
            }
            event_bus::emit(Event::OpenGameConfig { gid, first_run: true });
        });
    }

    pub fn save_game_config(&self, gid: u32, entry: GameEntry) {
        info!(target: LOG, "Saving configuration for {} ({gid})", entry.name);
        configuration::update(|c| c.games.insert(gid, entry));
    }

    pub fn launch_game(&self, id: &str) {
        info!(target: LOG, "Launching game with id {id}...");
        let _ = tabbed(|| Shell::get().run_command(&format!("steam steam://rungameid/{id}"), false));
    }

    /// Launch configuration for the `run` wrapper (by app id or overlay id).
    pub fn configuration(&self, id: &str) -> SteamGameConfig {
        let mut cfg = SteamGameConfig::default();
        let entry = configuration::read(|c| {
            id.parse::<u32>()
                .ok()
                .and_then(|gid| c.games.get(&gid).cloned())
                .or_else(|| c.games.values().find(|g| g.overlay_id.as_deref() == Some(id)).cloned())
        });

        if let Some(inhibit) = &self.which_systemd_inhibit {
            cfg.wrappers.push(inhibit.clone());
            cfg.wrappers.push("--who".into());
            let who = entry.as_ref().map(|e| e.name.clone()).unwrap_or_else(|| constants::APP_NAME.into());
            cfg.wrappers.push(format!("\"{who}\""));
        }

        let Some(game) = entry else {
            error!(target: LOG, "No configuration entry found for {id}");
            return cfg;
        };
        if let Some(env) = &game.env {
            cfg.environment.extend(utils::parse_env(env));
        }
        if let Some(args) = &game.args {
            cfg.parameters = args.clone();
        }
        if game.proton {
            cfg.environment.insert(
                "SteamDeck".into(),
                if game.device == ComputerType::SteamDeck { "1" } else { "0" }.into(),
            );
            if game.sync != WineSyncOption::Auto {
                let flag = |cond: bool| if cond { "0" } else { "1" }.to_string();
                cfg.environment
                    .insert("PROTON_USE_NTSYNC".into(), if game.sync == WineSyncOption::Ntsync { "1" } else { "0" }.into());
                cfg.environment.insert("PROTON_NO_NTSYNC".into(), flag(game.sync == WineSyncOption::Ntsync));
                cfg.environment.insert("PROTON_NO_FSYNC".into(), flag(game.sync == WineSyncOption::Fsync));
                cfg.environment.insert("PROTON_NO_ESYNC".into(), flag(game.sync == WineSyncOption::Esync));
            }
        }
        if let Some(gpu) = &game.gpu {
            cfg.environment.extend(hardware::get().gpu_selector_env(gpu));
        }
        if let Some(mangohud) = &self.which_mangohud
            && game.metrics_level != MangoHudLevel::NoDisplay
        {
            cfg.environment.insert("MANGOHUD_CONFIG".into(), format!("preset={}", game.metrics_level as u8));
            cfg.environment.insert("MANGOHUD_DLSYM".into(), "1".into());
            cfg.environment.insert("MANGOHUD".into(), "1".into());
            cfg.wrappers.push(mangohud.clone());
        }
        if let Some(wrappers) = &game.wrappers {
            cfg.wrappers.extend(wrappers.split(' ').filter(|w| !w.is_empty()).map(|w| w.to_string()));
        }
        cfg
    }

    fn image_path(gid: u32, suffix: &str) -> Option<String> {
        ["jpg", "png"]
            .iter()
            .map(|ext| format!("{}/{gid}{suffix}.{ext}", *constants::LOGOS_DIR))
            .find(|p| utils::files::exists(p))
    }

    pub fn icon(&self, gid: u32) -> Option<String> {
        Self::image_path(gid, "_logo")
    }

    pub fn banner(&self, gid: u32) -> Option<String> {
        Self::image_path(gid, "_hero")
    }

    // -----------------------------------------------------------------------------------------------------------------
    // Decky plugin
    // -----------------------------------------------------------------------------------------------------------------

    fn plugin_needs_update() -> bool {
        let installed = std::fs::read_to_string(constants::RCCDC_PACKAGE_FILE.as_str())
            .ok()
            .and_then(|c| serde_json::from_str::<serde_json::Value>(&c).ok())
            .and_then(|v| v.get("version").and_then(|v| v.as_str()).map(|s| s.to_string()))
            .unwrap_or_default();
        SemanticVersion::parse(constants::PLUGIN_VERSION) > SemanticVersion::parse(&installed)
    }

    /// Installs/updates the Decky plugin. On first install the GUI asks the user first
    /// (`Event` -> `answer_decky_install` command -> `install_rccdc(true)`).
    pub fn install_rccdc(&self, accepted: bool) {
        if !utils::files::exists(&constants::DECKY_SERVICE_PATH) {
            warn!(target: LOG, "No Decky installation found, skipping plugin installation");
            return;
        }
        if !utils::files::exists(&constants::RCCDC_PATH) {
            let asked = configuration::read(|c| c.application.asked_install_rccdc);
            if accepted || *constants::DEV_MODE {
                info!(target: LOG, "Installing plugin for first time");
                tabbed(Self::copy_plugin);
                configuration::update(|c| c.application.asked_install_rccdc = true);
            } else if !asked {
                event_bus::emit(Event::AskDeckyInstall);
            }
        } else {
            if Self::plugin_needs_update() {
                info!(target: LOG, "Updating Decky plugin");
                tabbed(Self::copy_plugin);
            } else {
                info!(target: LOG, "Plugin up to date");
            }
            configuration::update(|c| c.application.asked_install_rccdc = true);
        }
    }

    pub fn decline_rccdc(&self) {
        configuration::update(|c| c.application.asked_install_rccdc = true);
    }

    fn copy_plugin() {
        info!(target: LOG, "Copying plugin");
        thread::spawn(|| {
            if let Err(e) = utils::files::copy_path(&constants::RCCDC_ASSET_PATH, &constants::USER_PLUGIN_DIR) {
                error!(target: LOG, "Error while installing RCCDeckyCompanion plugin: {e}");
                return;
            }
            let rccdc = constants::RCCDC_PATH.as_str();
            let commands = [
                format!("if [[ -d \"{rccdc}\" ]]; then rm -R \"{rccdc}\"; fi"),
                format!("chmod 777 \"{}/..\"", *constants::PLUGINS_FOLDER),
                format!("mv \"{}\" \"{rccdc}\"", *constants::USER_PLUGIN_DIR),
                "systemctl restart plugin_loader.service".to_string(),
            ];
            for cmd in commands {
                let _ = Shell::get().run_elevated_command(&cmd, false);
            }
        });
    }
}
