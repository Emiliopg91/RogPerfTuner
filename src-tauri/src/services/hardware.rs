//! Hardware service: GPU environment, battery limit/status, panel overdrive,
//! boot sound and compatible USB device tracking.

use std::{
    collections::{BTreeMap, HashMap},
    sync::{LazyLock, Mutex},
    time::Instant,
};

use log::{error, info};

use crate::{
    clients::{
        dbus::notifications_client::TOASTER,
        file::{
            battery_charge_limit_client::set_charge_limit,
            battery_status_client::{is_on_battery, start_battery_monitor},
            firmware::asus_armoury::{boot_sound_client::BOOT_SOUND, panel_overdrive_client::PANEL_OVERDRIVE},
        },
        lib::usb_client as usb,
        tcp::open_rgb::open_rgb_client::OPENRGB,
    },
    features::features,
    models::hardware::{BatteryThreshold, UsbIdentifier},
    services::configuration,
    utils::{
        self, constants, elapsed_secs,
        event_bus::{self, Event},
        logger::tabbed,
        translator,
    },
};

const LOG: &str = "HardwareService";

struct State {
    charge_limit: BatteryThreshold,
    on_battery: bool,
    connected_devices: Vec<UsbIdentifier>,
}

pub struct HardwareService {
    state: Mutex<State>,
    action: Mutex<()>,
    /// GPU brand -> environment variables selecting it
    gpus: BTreeMap<String, String>,
}

static SERVICE: LazyLock<HardwareService> = LazyLock::new(HardwareService::new);

pub fn get() -> &'static HardwareService {
    &SERVICE
}

fn device_name(dev: &UsbIdentifier) -> String {
    OPENRGB
        .compatible_devices()
        .iter()
        .find(|d| *d == dev)
        .map(|d| d.name.clone())
        .unwrap_or_default()
}

fn is_compatible(dev: &UsbIdentifier) -> bool {
    !device_name(dev).is_empty()
}

/// Copies the Vulkan/OpenCL ICD files of the GPU so games can be pinned to it.
fn setup_gpu_env(brand: &str, base_env: &str) -> String {
    let _ = utils::files::remove_path(&constants::LIB_VK_DIR);
    let _ = std::fs::create_dir_all(constants::LIB_VK_DIR.as_str());
    let _ = utils::files::remove_path(&constants::LIB_OCL_DIR);
    let _ = std::fs::create_dir_all(constants::LIB_OCL_DIR.as_str());

    let mut env = format!("{base_env} ");
    let icd = if brand == "amd" { "radeon" } else { brand };
    let vk: Vec<String> = [format!("{icd}_icd.json"), format!("{icd}_icd.i686.json"), format!("{icd}_icd.x86_64.json")]
        .into_iter()
        .filter_map(|var| {
            let src = format!("{}{var}", constants::USR_SHARE_VK_DIR);
            let dst = format!("{}{var}", *constants::LIB_VK_DIR);
            (utils::files::exists(&src) && utils::files::copy_path(&src, &dst).is_ok()).then_some(dst)
        })
        .collect();
    if !vk.is_empty() {
        env.push_str(&format!("VK_ICD_FILENAMES={} ", vk.join(":")));
    }
    let ocl = if brand == "amd" { "amdocl64" } else { brand };
    let src = format!("{}{ocl}.icd", constants::USR_SHARE_OCL_DIR);
    let dst = format!("{}{ocl}.icd", *constants::LIB_OCL_DIR);
    if utils::files::exists(&src) && utils::files::copy_path(&src, &dst).is_ok() {
        env.push_str(&format!("OCL_ICD_FILENAMES={dst} "));
    }
    env.trim().to_string()
}

impl HardwareService {
    fn new() -> Self {
        info!(target: LOG, "Initializing HardwareService");
        let f = features();
        let service = tabbed(|| {
            info!(target: LOG, "Detected CPU");
            tabbed(|| {
                info!(target: LOG, "{}", f.cpu_name);
                tabbed(|| {
                    if f.ppt_pl1_spl.is_some() {
                        info!(target: LOG, "TDP control available");
                    }
                    if f.boost_control.is_some() {
                        info!(target: LOG, "Boost control available");
                    }
                });
            });

            let mut gpus = BTreeMap::new();
            if let Some(gpu) = &f.gpu {
                info!(target: LOG, "Detected GPU");
                tabbed(|| {
                    info!(target: LOG, "{}", gpu.name);
                    gpus.insert(gpu.brand.clone(), setup_gpu_env(&gpu.brand, &gpu.env));
                    tabbed(|| {
                        if f.nv_boost.is_some() {
                            info!(target: LOG, "Dynamic boost control available");
                        }
                        if f.nv_thermal.is_some() {
                            info!(target: LOG, "Throttle temperature control available");
                        }
                    });
                });
            }

            let charge_limit = configuration::read(|c| c.platform.charge_limit);
            if f.bat_limit.is_some() {
                info!(target: LOG, "Getting battery charge limit");
                tabbed(|| {
                    if let Err(e) = set_charge_limit(charge_limit) {
                        error!(target: LOG, "Error setting charge limit: {e}");
                    }
                    info!(target: LOG, "{}%", charge_limit.value());
                });
            }

            HardwareService {
                state: Mutex::new(State {
                    charge_limit,
                    on_battery: is_on_battery(),
                    connected_devices: usb::usb_devices(&is_compatible),
                }),
                action: Mutex::new(()),
                gpus,
            }
        });
        service.set_panel_overdrive(false);
        service
    }

    /// Hooks the service to the event bus; called once after construction.
    pub fn start(&'static self) {
        event_bus::subscribe(|e| match e {
            Event::DeviceChanged => get().on_device_event(),
            Event::BatteryStatus(on_bat) => get().on_battery_event(*on_bat),
            _ => {}
        });
        usb::start_monitor();
        start_battery_monitor();
    }

    fn on_device_event(&self) {
        let previous = self.state.lock().unwrap().connected_devices.clone();
        let (current, added, removed) = usb::compare_connected_devs(&previous, &is_compatible);
        if !added.is_empty() {
            info!(target: LOG, "Added compatible device(s):");
            tabbed(|| added.iter().for_each(|d| info!(target: LOG, "{}", device_name(d))));
        }
        if !removed.is_empty() {
            info!(target: LOG, "Removed compatible device(s):");
            tabbed(|| removed.iter().for_each(|d| info!(target: LOG, "{}", device_name(d))));
        }
        self.state.lock().unwrap().connected_devices = current;
        if !added.is_empty() {
            event_bus::emit(Event::UsbAdded);
        } else if !removed.is_empty() {
            event_bus::emit(Event::UsbRemoved(removed));
        }
    }

    fn on_battery_event(&self, on_bat: bool) {
        self.state.lock().unwrap().on_battery = on_bat;
        self.set_panel_overdrive(false);
        info!(
            target: LOG,
            "AC {}plugged, battery {}engaged",
            if on_bat { "un" } else { "" },
            if on_bat { "" } else { "dis" }
        );
        tabbed(|| event_bus::emit(Event::OnBattery(on_bat)));
    }

    pub fn on_battery(&self) -> bool {
        self.state.lock().unwrap().on_battery
    }

    pub fn charge_threshold(&self) -> Option<BatteryThreshold> {
        features().bat_limit.as_ref().map(|_| self.state.lock().unwrap().charge_limit)
    }

    pub fn set_charge_threshold(&self, threshold: BatteryThreshold) -> Result<(), String> {
        let _guard = self.action.lock().unwrap();
        if self.state.lock().unwrap().charge_limit == threshold {
            return Ok(());
        }
        info!(target: LOG, "Setting charge limit to {}%", threshold.value());
        let t0 = Instant::now();
        set_charge_limit(threshold).map_err(|e| e.to_string())?;
        self.state.lock().unwrap().charge_limit = threshold;
        configuration::update(|c| c.platform.charge_limit = threshold);
        info!(target: LOG, "Charge limit setted after {} seconds", elapsed_secs(t0));
        TOASTER.show_toast(&translator::translate_with(
            "applied.battery.threshold",
            &[("value", &threshold.value().to_string())],
        ));
        event_bus::emit(Event::ChargeThreshold(threshold));
        Ok(())
    }

    pub fn set_panel_overdrive(&self, enable: bool) {
        if let Some(client) = PANEL_OVERDRIVE.as_ref() {
            info!(target: LOG, "Panel Overdrive: {}", if enable { "Enabled" } else { "Disabled" });
            tabbed(|| {
                if let Err(e) = client.set_current_value(enable as i32) {
                    error!(target: LOG, "{e}");
                }
            });
        }
    }

    pub fn gpus(&self) -> &BTreeMap<String, String> {
        &self.gpus
    }

    pub fn gpu_selector_env(&self, gpu: &str) -> HashMap<String, String> {
        self.gpus.get(gpu).map(|env| utils::parse_env(env)).unwrap_or_default()
    }

    pub fn boot_sound(&self) -> Option<bool> {
        BOOT_SOUND.as_ref().and_then(|c| c.current_value().ok()).map(|v| v != 0)
    }

    pub fn set_boot_sound(&self, enable: bool) -> Result<(), String> {
        let Some(client) = BOOT_SOUND.as_ref() else {
            return Err("Boot sound not available".into());
        };
        if self.boot_sound() == Some(enable) {
            return Ok(());
        }
        let _guard = self.action.lock().unwrap();
        info!(target: LOG, "Setting boot sound: {}", if enable { "Enabled" } else { "Disabled" });
        let t0 = Instant::now();
        tabbed(|| client.set_current_value(enable as i32)).map_err(|e| e.to_string())?;
        info!(target: LOG, "Boot sound setted after {} seconds", elapsed_secs(t0));
        event_bus::emit(Event::BootSound(enable));
        Ok(())
    }
}
