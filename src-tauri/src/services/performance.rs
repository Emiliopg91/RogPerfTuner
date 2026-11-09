//! Performance service: profiles (with the SMART auto mode), TDPs, Nvidia limits,
//! CPU schedulers (EEVDF/BORE/sched_ext), NVMe schedulers and fan curves.

use std::{
    collections::{BTreeMap, HashMap},
    sync::{
        Arc, LazyLock, Mutex,
        atomic::{AtomicBool, Ordering},
    },
    thread::{self, JoinHandle},
    time::Instant,
};

use log::{debug, error, info, warn};

use crate::{
    clients::{
        dbus::{notifications_client::TOASTER, platform_client::PLATFORM},
        file::{
            battery_status_client::is_on_battery,
            boost_control_client::set_boost,
            firmware::asus_armoury::{
                ArmouryClient, nv_boost_client::NV_BOOST, nv_temp_client::NV_TEMP, pl1_spl_client::PL1_SPL,
                pl2_sppt_client::PL2_SPPT, pl3_fppt_client::PL3_FPPT,
            },
            power_profile_client::set_power_profile,
            scaling_governor_client::set_governor,
            sched_bore_client::SCHED_BORE,
            ssd_scheduler_client::SSD_SCHEDULER,
        },
        shell::{asusctl_client::ASUSCTL, scxctl_client::SCXCTL},
    },
    features::features,
    models::{
        hardware::FanCurveData,
        performance::{CpuGovernor, PerformanceProfile, PlatformProfile, PowerProfile},
    },
    services::configuration,
    utils::{
        self, capitalize, elapsed_secs,
        cpu::cpu_use_rate,
        event_bus::{self, Event},
        logger::{add_tab, rem_tab, tabbed},
        sleep_ms, translator,
    },
};

const LOG: &str = "PerformanceService";
const CPU_PRIORITY: i32 = -17;
const IO_CLASS: u8 = 2;
const IO_PRIORITY: u8 = ((CPU_PRIORITY + 20) / 5) as u8;

struct State {
    on_battery: bool,
    current_profile: PerformanceProfile,
    current_scheduler: String,
    current_ssd_scheduler: String,
}

pub struct PerformanceService {
    state: Mutex<State>,
    actual_profile: Mutex<PerformanceProfile>,
    per_prof: Mutex<()>,
    stop_flag: Arc<AtomicBool>,
    smart_thread: Mutex<Option<JoinHandle<()>>>,
    default_scheduler: String,
    available_schedulers: Vec<String>,
}

static SERVICE: LazyLock<PerformanceService> = LazyLock::new(PerformanceService::new);

pub fn get() -> &'static PerformanceService {
    &SERVICE
}

impl PerformanceService {
    fn new() -> Self {
        info!(target: LOG, "Initializing PerformanceService");
        tabbed(|| {
            let f = features();
            let (current_profile, configured_sched) =
                configuration::read(|c| (c.platform.performance.profile, c.platform.performance.scheduler.clone()));

            let on_battery = is_on_battery();
            if f.bat_status.is_some() {
                info!(target: LOG, "Laptop on {} mode", if on_battery { "battery" } else { "AC" });
            }

            let mut available_schedulers = vec!["EEVDF".to_string()];
            if SCHED_BORE.available() {
                available_schedulers.push("BORE".into());
            }
            if SCXCTL.available() {
                available_schedulers.extend(SCXCTL.available_schedulers().iter().map(|s| capitalize(s)));
            }
            let mut default_scheduler = "EEVDF".to_string();
            if SCHED_BORE.available() && SCHED_BORE.read().map(|v| v.trim() == "1").unwrap_or(false) {
                default_scheduler = "BORE".into();
            }
            let mut current_scheduler = configured_sched.unwrap_or_default();
            if current_scheduler.is_empty() || !available_schedulers.contains(&current_scheduler) {
                current_scheduler = default_scheduler.clone();
                configuration::update(|c| c.platform.performance.scheduler = Some(current_scheduler.clone()));
            }

            if f.fan_control {
                Self::init_fan_curves();
            }

            info!(target: LOG, "Available schedulers:");
            tabbed(|| info!(target: LOG, "{}", available_schedulers.join(", ")));
            if SSD_SCHEDULER.available() {
                info!(target: LOG, "Available SSD schedulers:");
                tabbed(|| info!(target: LOG, "{}", SSD_SCHEDULER.available_schedulers().join(", ")));
            }

            for res in [
                PLATFORM.set_change_platform_profile_on_ac(false),
                PLATFORM.set_change_platform_profile_on_battery(false),
                PLATFORM.set_platform_profile_linked_epp(false),
            ] {
                if let Err(e) = res {
                    error!(target: LOG, "Error configuring asusd: {e}");
                }
            }

            PerformanceService {
                state: Mutex::new(State {
                    on_battery,
                    current_profile,
                    current_scheduler,
                    current_ssd_scheduler: "none".into(),
                }),
                actual_profile: Mutex::new(PerformanceProfile::Performance),
                per_prof: Mutex::new(()),
                stop_flag: Arc::new(AtomicBool::new(true)),
                smart_thread: Mutex::new(None),
                default_scheduler,
                available_schedulers,
            }
        })
    }

    fn init_fan_curves() {
        info!(target: LOG, "Available fans:");
        tabbed(|| {
            for profile in PerformanceProfile::ALL {
                let platform = profile.platform_profile();
                let existing = configuration::read(|c| c.platform.curves.get(profile.as_str()).cloned());
                match existing {
                    None => {
                        let _ = ASUSCTL.set_curves_to_defaults(platform);
                        let data = ASUSCTL.fan_curve_data(platform).unwrap_or_default();
                        configuration::update(|c| {
                            let entry = c.platform.curves.entry(profile.as_str().to_string()).or_default();
                            for (fan, mut curve) in data {
                                let fc = entry.entry(fan).or_default();
                                fc.presets = curve.to_data();
                                curve.normalize();
                                fc.current = curve.to_data();
                            }
                        });
                    }
                    Some(curves) => {
                        for (fan, curve) in curves {
                            let _ = ASUSCTL.set_fan_curve_string_data(platform, &fan, &curve.current);
                        }
                    }
                }
                // Keep custom curves enabled on every profile: asusctl >= 6.4 needs them for PPT tuning
                let _ = ASUSCTL.set_fan_curves_enabled(platform, true);
            }
            let fans = configuration::read(|c| {
                c.platform.curves.values().next().map(|m| m.keys().cloned().collect::<Vec<_>>()).unwrap_or_default()
            });
            info!(target: LOG, "{}", fans.join(", "));
        });
    }

    /// Hooks the service to the event bus and applies the stored settings.
    pub fn start(&'static self) {
        event_bus::subscribe(|e| match e {
            Event::OnBattery(on_bat) => {
                get().state.lock().unwrap().on_battery = *on_bat;
                get().restore();
            }
            Event::ApplicationShutdown => {
                get().set_performance_profile(PerformanceProfile::Performance, true, true, false);
            }
            _ => {}
        });
        self.restore();
    }

    fn on_battery(&self) -> bool {
        self.state.lock().unwrap().on_battery
    }

    pub fn renice(&self, pid: i32) {
        info!(target: LOG, "Renicing process {pid}");
        tabbed(|| loop {
            let ion = utils::process::renice_hierarchy(pid, CPU_PRIORITY);
            let ren = utils::process::ionice_hierarchy(pid, IO_CLASS, IO_PRIORITY);
            if ion == ren {
                break;
            }
        });
    }

    pub fn performance_profile(&self) -> PerformanceProfile {
        self.state.lock().unwrap().current_profile
    }

    fn set_actual_performance_profile(&self, profile: PerformanceProfile) {
        let mut actual = self.actual_profile.lock().unwrap();
        info!(target: LOG, "Applying {} profile", profile.name());
        let t0 = Instant::now();
        tabbed(|| {
            // Fan curves must be enabled before changing platform profile: newer asusd/kernel
            // require an active custom fan curve to enable PPT tuning and accept PPT writes
            if features().fan_control {
                self.set_fan_curves(profile);
            }
            // power-profiles-daemon may write platform_profile, which makes the kernel disable custom
            // fan curves. Apply it before the platform profile so asusd's re-apply of fan curves comes last
            self.set_power_profile(profile);
            self.set_platform_profile(profile);
            self.set_boost();
            self.set_cpu_governor();
            self.set_tdps(profile);
            self.set_nvidia_profile(profile);
            info!(target: LOG, "Profile applied after {} seconds", elapsed_secs(t0));
        });
        *actual = profile;
    }

    fn next_smart(&self, samples: usize, level: i32) -> PerformanceProfile {
        let actual = *self.actual_profile.lock().unwrap();
        let mut buffer = vec![];
        for _ in 0..samples {
            if self.stop_flag.load(Ordering::SeqCst) {
                return actual;
            }
            buffer.push((cpu_use_rate(1000) * 100.0).round());
        }
        let avg = (buffer.iter().sum::<f64>() / samples as f64).round();
        debug!(target: LOG, "Average CPU usage: {avg}%");
        if avg > 67.0 {
            let next = actual.next(false, false);
            if next != actual {
                info!(target: LOG, "Ramp up to {}", next.name());
            }
            next
        } else if avg < 25.0 {
            let next = actual.previous(false);
            if next != actual {
                if level != 0 {
                    debug!(target: LOG, "Waiting for {level} more checks before ramp down");
                    return self.next_smart(samples, level - 1);
                }
                info!(target: LOG, "Ramp down to {}", next.name());
            }
            next
        } else {
            actual
        }
    }

    fn smart_worker(&'static self) {
        while !self.stop_flag.load(Ordering::SeqCst) {
            let next = self.next_smart(5, 2);
            if !self.stop_flag.load(Ordering::SeqCst) && next != *self.actual_profile.lock().unwrap() {
                tabbed(|| self.set_actual_performance_profile(next));
            }
        }
    }

    fn stop_smart(&self) {
        self.stop_flag.store(true, Ordering::SeqCst);
        if let Some(handle) = self.smart_thread.lock().unwrap().take() {
            info!(target: LOG, "Waiting for {} worker to stop", PerformanceProfile::Smart.name());
            let _ = handle.join();
        }
    }

    pub fn set_performance_profile(&'static self, profile: PerformanceProfile, temporal: bool, force: bool, toast: bool) {
        let _guard = self.per_prof.lock().unwrap();
        if profile != self.performance_profile() || force {
            info!(target: LOG, "Setting {} profile", profile.name());
            add_tab();
            if profile != PerformanceProfile::Smart {
                self.stop_smart();
                self.set_actual_performance_profile(profile);
            } else {
                self.stop_smart();
                info!(target: LOG, "Starting {} worker", PerformanceProfile::Smart.name());
                self.set_actual_performance_profile(PerformanceProfile::Performance);
                self.stop_flag.store(false, Ordering::SeqCst);
                *self.smart_thread.lock().unwrap() = Some(thread::spawn(|| get().smart_worker()));
            }
            if !temporal {
                configuration::update(|c| c.platform.performance.profile = profile);
            }
            if toast {
                let label = translator::translate(&format!("label.profile.{}", profile.name())).to_lowercase();
                TOASTER.show_toast(&translator::translate_with("profile.applied", &[("profile", &label)]));
            }
            self.state.lock().unwrap().current_profile = profile;
            rem_tab();
        } else {
            info!(target: LOG, "Profile {} already setted", profile.name().to_lowercase());
        }
        event_bus::emit(Event::PerformanceProfile(profile));
    }

    fn set_platform_profile(&self, profile: PerformanceProfile) {
        if self.on_battery() {
            info!(target: LOG, "Platform profile: Not available on battery");
            return;
        }
        let platform = profile.platform_profile();
        info!(target: LOG, "Platform profile: {}", platform.name());
        tabbed(|| {
            if let Err(e) = PLATFORM.set_platform_profile(platform) {
                error!(target: LOG, "Error while setting platform profile: {e}");
            }
            if let Err(e) = PLATFORM.set_enable_ppt_group(true) {
                warn!(target: LOG, "Error while enabling PPT tuning: {e}");
            }
        });
    }

    fn set_boost(&self) {
        if features().boost_control.is_none() {
            return;
        }
        // Boost is always on while on AC and off on battery
        let enabled = !self.on_battery();
        info!(target: LOG, "CPU boost: {}", if enabled { "ON" } else { "OFF" });
        tabbed(|| {
            if let Err(e) = set_boost(enabled) {
                error!(target: LOG, "Error while setting CPU boost: {e}");
            }
        });
    }

    fn set_cpu_governor(&self) {
        if features().scaling_governor.is_none() {
            return;
        }
        let governor = if self.on_battery() { CpuGovernor::Powersave } else { CpuGovernor::Performance };
        info!(target: LOG, "CPU governor: {}", governor.as_str().to_uppercase());
        tabbed(|| {
            if let Err(e) = set_governor(governor) {
                error!(target: LOG, "Error while setting CPU governor: {e}");
            }
        });
    }

    fn set_power_profile(&self, profile: PerformanceProfile) {
        if features().acpi_profile.is_none() {
            return;
        }
        let power = if !self.on_battery() { PowerProfile::Performance } else { profile.power_profile() };
        info!(target: LOG, "Power profile: {}", power.as_str().to_uppercase());
        tabbed(|| {
            if let Err(e) = set_power_profile(power) {
                error!(target: LOG, "Error while setting power profile: {e}");
            }
        });
    }

    /// TDP for a profile: 100% / 80% / 60% of the maximum.
    fn scaled_tdp(client: &ArmouryClient, profile: PerformanceProfile) -> Result<i32, String> {
        let max = client.max_value().map_err(|e| e.to_string())? as f64;
        Ok(match profile {
            PerformanceProfile::Performance => max as i32,
            PerformanceProfile::Balanced => (max * 0.8) as i32,
            PerformanceProfile::Quiet => (max * 0.6) as i32,
            PerformanceProfile::Smart => client.current_value().map_err(|e| e.to_string())?,
        })
    }

    fn set_tdps(&self, profile: PerformanceProfile) {
        let Some(pl1) = PL1_SPL.as_ref() else {
            return;
        };
        info!(target: LOG, "TDP values");
        tabbed(|| {
            let res: Result<(), String> = (|| {
                let v = Self::scaled_tdp(pl1, profile)?;
                info!(target: LOG, "PL1: {v}W");
                pl1.set_current_value(v).map_err(|e| e.to_string())?;
                sleep_ms(25);
                if let Some(pl2) = PL2_SPPT.as_ref() {
                    let v = Self::scaled_tdp(pl2, profile)?;
                    info!(target: LOG, "PL2: {v}W");
                    pl2.set_current_value(v).map_err(|e| e.to_string())?;
                    sleep_ms(25);
                    if let Some(pl3) = PL3_FPPT.as_ref() {
                        let v = Self::scaled_tdp(pl3, profile)?;
                        info!(target: LOG, "PL3: {v}W");
                        pl3.set_current_value(v).map_err(|e| e.to_string())?;
                        sleep_ms(25);
                    }
                }
                Ok(())
            })();
            if let Err(e) = res {
                error!(target: LOG, "Error setting CPU TDPs: {e}");
            }
        });
    }

    fn min_max(client: &ArmouryClient) -> Result<(i32, i32), String> {
        Ok((
            client.min_value().map_err(|e| e.to_string())?,
            client.max_value().map_err(|e| e.to_string())?,
        ))
    }

    fn set_nvidia_profile(&self, profile: PerformanceProfile) {
        if NV_BOOST.is_none() && NV_TEMP.is_none() {
            return;
        }
        let on_battery = self.on_battery();
        info!(target: LOG, "Nvidia GPU");
        tabbed(|| {
            if let Some(nvb) = NV_BOOST.as_ref() {
                let res: Result<(), String> = (|| {
                    let (min, max) = Self::min_max(nvb)?;
                    if on_battery && min == max {
                        info!(target: LOG, "Dynamic Boost: Not available on battery");
                        return Ok(());
                    }
                    let v = match profile {
                        PerformanceProfile::Performance => max,
                        PerformanceProfile::Balanced => (max + min) / 2,
                        PerformanceProfile::Quiet => min,
                        PerformanceProfile::Smart => nvb.current_value().map_err(|e| e.to_string())?,
                    };
                    info!(target: LOG, "Dynamic Boost: {v}W");
                    nvb.set_current_value(v).map_err(|e| e.to_string())?;
                    sleep_ms(25);
                    Ok(())
                })();
                if let Err(e) = res {
                    error!(target: LOG, "Error setting Nvidia Boost: {e}");
                }
            }
            if let Some(nvt) = NV_TEMP.as_ref() {
                let res: Result<(), String> = (|| {
                    let (min, max) = Self::min_max(nvt)?;
                    let v = if !on_battery {
                        max
                    } else {
                        match profile {
                            PerformanceProfile::Performance => max,
                            PerformanceProfile::Balanced => (max + min) / 2,
                            PerformanceProfile::Quiet => min,
                            PerformanceProfile::Smart => nvt.current_value().map_err(|e| e.to_string())?,
                        }
                    };
                    info!(target: LOG, "Throttle temp: {v}ºC");
                    nvt.set_current_value(v).map_err(|e| e.to_string())
                })();
                if let Err(e) = res {
                    error!(target: LOG, "Error setting Nvidia throttle temp: {e}");
                }
            }
        });
    }

    pub fn restore(&'static self) {
        if self.on_battery() {
            self.set_performance_profile(PerformanceProfile::Quiet, true, true, true);
        } else {
            let profile = configuration::read(|c| c.platform.performance.profile);
            self.set_performance_profile(profile, false, true, true);
        }
        let current = self.current_scheduler();
        let (sched, ssd) = configuration::read(|c| {
            (
                c.platform.performance.scheduler.clone().unwrap_or(current),
                c.platform.performance.ssd_scheduler.clone(),
            )
        });
        self.set_scheduler(&sched, false);
        self.set_ssd_scheduler(&ssd, false);
    }

    pub fn next_performance_profile(&'static self) -> PerformanceProfile {
        let next = self.performance_profile().next(true, true);
        self.set_performance_profile(next, false, false, true);
        next
    }

    pub fn available_schedulers(&self) -> Vec<String> {
        self.available_schedulers.clone()
    }

    pub fn current_scheduler(&self) -> String {
        self.state.lock().unwrap().current_scheduler.clone()
    }

    pub fn default_scheduler(&self) -> String {
        self.default_scheduler.clone()
    }

    pub fn set_scheduler(&self, scheduler: &str, temporal: bool) {
        info!(target: LOG, "Applying {scheduler} scheduler");
        tabbed(|| {
            let current = self.current_scheduler();
            if scheduler == current {
                info!(target: LOG, "Scheduler already applied");
                return;
            }
            let t0 = Instant::now();
            let kernel = |s: &str| s == "EEVDF" || s == "BORE";
            if kernel(scheduler) && SCXCTL.available() {
                SCXCTL.stop();
            }
            if scheduler == "EEVDF" {
                if SCHED_BORE.available() {
                    let _ = SCHED_BORE.write("0");
                }
            } else if scheduler == "BORE" {
                let _ = SCHED_BORE.write("1");
            } else {
                SCXCTL.start(&scheduler.to_lowercase(), self.on_battery());
            }
            self.state.lock().unwrap().current_scheduler = scheduler.to_string();
            if !temporal {
                configuration::update(|c| c.platform.performance.scheduler = Some(scheduler.to_string()));
            }
            info!(target: LOG, "Scheduler applied after {} seconds", elapsed_secs(t0));
        });
        event_bus::emit(Event::Scheduler(Some(scheduler.to_string())));
    }

    pub fn available_ssd_schedulers(&self) -> Vec<String> {
        if SSD_SCHEDULER.available() { SSD_SCHEDULER.available_schedulers() } else { vec![] }
    }

    pub fn current_ssd_scheduler(&self) -> Option<String> {
        SSD_SCHEDULER
            .available()
            .then(|| self.state.lock().unwrap().current_ssd_scheduler.clone())
    }

    pub fn set_ssd_scheduler(&self, scheduler: &str, temporal: bool) {
        if !SSD_SCHEDULER.available() {
            return;
        }
        info!(target: LOG, "Applying {scheduler} SSD scheduler");
        tabbed(|| {
            if self.state.lock().unwrap().current_ssd_scheduler == scheduler {
                info!(target: LOG, "Scheduler already applied");
                return;
            }
            let t0 = Instant::now();
            if let Err(e) = SSD_SCHEDULER.set_scheduler(scheduler) {
                error!(target: LOG, "Error applying SSD scheduler: {e}");
            }
            self.state.lock().unwrap().current_ssd_scheduler = scheduler.to_string();
            if !temporal {
                configuration::update(|c| c.platform.performance.ssd_scheduler = scheduler.to_string());
            }
            info!(target: LOG, "Scheduler applied after {} seconds", elapsed_secs(t0));
        });
        event_bus::emit(Event::SsdScheduler(scheduler.to_string()));
    }

    // -----------------------------------------------------------------------------------------------------------------
    // Fan curves
    // -----------------------------------------------------------------------------------------------------------------

    pub fn fans(&self) -> Vec<String> {
        let actual = *self.actual_profile.lock().unwrap();
        let mut fans = ASUSCTL.fans(actual.platform_profile()).unwrap_or_default();
        if fans.is_empty() {
            fans = configuration::read(|c| {
                c.platform.curves.values().next().map(|m| m.keys().cloned().collect()).unwrap_or_default()
            });
        }
        fans
    }

    pub fn fan_curve(&self, fan: &str, profile: &str) -> FanCurveData {
        configuration::read(|c| {
            c.platform
                .curves
                .get(profile)
                .and_then(|m| m.get(fan))
                .map(|fc| FanCurveData::from_data(&fc.current))
                .unwrap_or_default()
        })
    }

    pub fn default_fan_curve(&self, fan: &str, profile: &str) -> FanCurveData {
        configuration::read(|c| {
            c.platform
                .curves
                .get(profile)
                .and_then(|m| m.get(fan))
                .map(|fc| FanCurveData::from_data(&fc.presets))
                .unwrap_or_default()
        })
    }

    /// profile -> fan -> curve
    pub fn save_fan_curves(&self, curves: BTreeMap<String, HashMap<String, FanCurveData>>) {
        info!(target: LOG, "Saving curves");
        tabbed(|| {
            for (profile, curves) in &curves {
                info!(target: LOG, "Saving curves for {}", profile.to_uppercase());
                tabbed(|| {
                    let platform = PlatformProfile::from_str(profile).unwrap_or(PlatformProfile::Performance);
                    for (fan, curve) in curves {
                        let data = curve.to_data();
                        info!(target: LOG, "{fan}: {data}");
                        let _ = ASUSCTL.set_fan_curve_string_data(platform, fan, &data);
                        configuration::update(|c| {
                            c.platform
                                .curves
                                .entry(profile.clone())
                                .or_default()
                                .entry(fan.clone())
                                .or_default()
                                .current = data.clone();
                        });
                    }
                });
            }
        });
        info!(target: LOG, "Fan curved updated succesfully");
        info!(target: LOG, "Applying curve");
        let actual = *self.actual_profile.lock().unwrap();
        tabbed(|| self.set_fan_curves(actual));
        info!(target: LOG, "Curve applied");
    }

    fn set_fan_curves(&self, profile: PerformanceProfile) {
        let platform = profile.platform_profile();
        info!(target: LOG, "Fan profile: {}", platform.name());
        tabbed(|| {
            for (fan, data) in configuration::read(|c| c.platform.curves.get(profile.as_str()).cloned().unwrap_or_default()) {
                info!(target: LOG, "{fan}: {}", data.current.replace(',', " "));
            }
            if let Err(e) = ASUSCTL.set_fan_curves_enabled(platform, true) {
                error!(target: LOG, "Error while setting fan curve: {e}");
            }
        });
    }
}
