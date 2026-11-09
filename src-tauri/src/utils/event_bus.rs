//! Synchronous, typed event bus between services.
//! Handlers run on the emitting thread, in subscription order.

use std::sync::{Arc, LazyLock, RwLock};

use crate::models::{
    hardware::{BatteryThreshold, RgbBrightness, UsbIdentifier},
    performance::PerformanceProfile,
};

/// Payloads are informative for most subscribers (the GUI reloads the whole state).
#[allow(dead_code)]
#[derive(Debug, Clone)]
pub enum Event {
    ApplicationShutdown,
    ApplicationStop,
    /// A USB device was plugged or unplugged
    DeviceChanged,
    UsbAdded,
    UsbRemoved(Vec<UsbIdentifier>),
    /// Battery mode as seen by the rest of the services (muted while gaming)
    OnBattery(bool),
    /// Raw battery status from the polling client
    BatteryStatus(bool),
    ChargeThreshold(BatteryThreshold),
    BootSound(bool),
    RgbBrightness(RgbBrightness),
    RgbEffect(String),
    RgbColor(Option<String>),
    PerformanceProfile(PerformanceProfile),
    Scheduler(Option<String>),
    SsdScheduler(String),
    GamesChanged(usize),
    SteamConnected,
    SteamDisconnected,
    SteamGameLaunch { id: u32, name: String, pid: i32 },
    SteamGameStop { id: u32, name: String },
    /// Asks the GUI to show the game configuration dialog
    OpenGameConfig { gid: u32, first_run: bool },
    ShowGui,
    /// Asks the GUI whether the Decky plugin should be installed
    AskDeckyInstall,
}

type Handler = Arc<dyn Fn(&Event) + Send + Sync>;

static HANDLERS: LazyLock<RwLock<Vec<Handler>>> = LazyLock::new(|| RwLock::new(Vec::new()));

pub fn subscribe(handler: impl Fn(&Event) + Send + Sync + 'static) {
    HANDLERS.write().unwrap().push(Arc::new(handler));
}

pub fn emit(event: Event) {
    let handlers: Vec<Handler> = HANDLERS.read().unwrap().clone();
    for handler in handlers {
        handler(&event);
    }
}
