//! Battery status polling.

use std::{fs, thread};

use crate::{
    features::features,
    utils::{
        event_bus::{self, Event},
        sleep_ms,
    },
};

pub fn is_on_battery() -> bool {
    features()
        .bat_status
        .as_deref()
        .and_then(|p| fs::read_to_string(p).ok())
        .map(|s| s.trim() == "Discharging")
        .unwrap_or(false)
}

/// Polls the battery status every second, emitting `Event::BatteryStatus` on changes.
pub fn start_battery_monitor() {
    if features().bat_status.is_none() {
        return;
    }
    thread::spawn(|| {
        let mut last = is_on_battery();
        loop {
            sleep_ms(1000);
            let current = is_on_battery();
            if current != last {
                last = current;
                event_bus::emit(Event::BatteryStatus(current));
            }
        }
    });
}
