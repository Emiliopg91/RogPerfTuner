//! USB device discovery, without libudev.
//! Devices are read from sysfs and a watcher thread emits `Event::DeviceChanged` when the set changes,
//! which avoids linking against libudev.

use std::{fs, path::Path, thread};

use crate::{
    models::hardware::UsbIdentifier,
    utils::{
        event_bus::{self, Event},
        sleep_ms,
    },
};

fn read_attr(dir: &Path, attr: &str) -> Option<String> {
    fs::read_to_string(dir.join(attr)).ok().map(|s| s.trim().to_string())
}

/// Connected USB devices (not interfaces) accepted by `filter`.
pub fn usb_devices(filter: &dyn Fn(&UsbIdentifier) -> bool) -> Vec<UsbIdentifier> {
    let mut devices = vec![];
    if let Ok(rd) = fs::read_dir("/sys/bus/usb/devices") {
        for entry in rd.flatten() {
            let dir = entry.path();
            if let (Some(vendor), Some(product)) = (read_attr(&dir, "idVendor"), read_attr(&dir, "idProduct")) {
                let dev = UsbIdentifier {
                    id_vendor: vendor,
                    id_product: product,
                    name: read_attr(&dir, "product").unwrap_or_else(|| "Unknown USB device".into()),
                };
                if filter(&dev) && !devices.contains(&dev) {
                    devices.push(dev);
                }
            }
        }
    }
    devices
}

/// Returns `(current, added, removed)` compared to `previous`.
pub fn compare_connected_devs(
    previous: &[UsbIdentifier],
    filter: &dyn Fn(&UsbIdentifier) -> bool,
) -> (Vec<UsbIdentifier>, Vec<UsbIdentifier>, Vec<UsbIdentifier>) {
    let current = usb_devices(filter);
    let added = current.iter().filter(|d| !previous.contains(d)).cloned().collect();
    let removed = previous.iter().filter(|d| !current.contains(d)).cloned().collect();
    (current, added, removed)
}

/// Starts watching for USB plug/unplug events.
pub fn start_monitor() {
    thread::spawn(|| {
        let snapshot = || {
            let mut ids: Vec<String> = usb_devices(&|_| true)
                .into_iter()
                .map(|d| format!("{}:{}", d.id_vendor, d.id_product))
                .collect();
            ids.sort();
            ids
        };
        let mut last = snapshot();
        loop {
            sleep_ms(1500);
            let current = snapshot();
            if current != last {
                last = current;
                event_bus::emit(Event::DeviceChanged);
            }
        }
    });
}
