//! OpenRGB client: runs the bundled OpenRGB server and drives it
//! through the SDK protocol.

use std::{
    fs::{self, OpenOptions},
    net::TcpListener,
    os::unix::process::CommandExt,
    process::{Command, Stdio},
    sync::{Arc, LazyLock, Mutex, mpsc::channel},
    thread::{self, JoinHandle},
};

use log::{debug, error, info, warn};
use regex::Regex;

use crate::{
    clients::{
        shell::asusctl_client::ASUSCTL,
        tcp::open_rgb::{
            effects::{Devices, Effect},
            protocol::{self, Color, OrgbConnection},
        },
    },
    models::hardware::{RgbBrightness, UsbIdentifier},
    utils::{constants, logger::tabbed, sleep_ms},
};

const LOG: &str = "OpenRgbClient";

struct State {
    /// OpenRGB server PID and the thread waiting for it
    process: Option<(u32, JoinHandle<()>)>,
    conn: Option<Arc<OrgbConnection>>,
    devices: Devices,
    current_effect: usize,
}

pub struct OpenRgbClient {
    compatible_devices: Vec<UsbIdentifier>,
    effects: Vec<Effect>,
    state: Mutex<State>,
}

pub static OPENRGB: LazyLock<OpenRgbClient> = LazyLock::new(OpenRgbClient::new);

fn free_port() -> u16 {
    TcpListener::bind("127.0.0.1:0")
        .and_then(|l| l.local_addr())
        .map(|a| a.port())
        .unwrap_or(6742)
}

fn port_in_use(port: u16) -> bool {
    TcpListener::bind(("0.0.0.0", port)).is_err()
}

/// Waits until OpenRGB logs the end of device detection (up to 20 seconds).
fn wait_for_detection(log_path: &str) {
    let t0 = std::time::Instant::now();
    while t0.elapsed().as_secs() < 20 {
        if fs::read_to_string(log_path).is_ok_and(|log| log.contains("Detection completed")) {
            debug!(target: LOG, "Device detection completed after {} ms", t0.elapsed().as_millis());
            return;
        }
        sleep_ms(100);
    }
    warn!(target: LOG, "Timeout waiting for OpenRGB device detection");
}

/// Requests the device list until its size stops changing, in case detection is still running.
fn request_devices_stable(conn: &OrgbConnection) -> Result<Vec<protocol::Device>, String> {
    let mut devices = conn.request_devices()?;
    for _ in 0..10 {
        sleep_ms(300);
        let again = conn.request_devices()?;
        if again.len() == devices.len() {
            break;
        }
        debug!(target: LOG, "Device list changed from {} to {} devices", devices.len(), again.len());
        devices = again;
    }
    Ok(devices)
}

impl OpenRgbClient {
    fn new() -> Self {
        info!(target: LOG, "Reading UDEV files");
        let compatible_devices = tabbed(|| {
            let re = Regex::new(
                r#"SUBSYSTEMS==".*?", ATTRS\{idVendor\}=="([0-9a-fA-F]+)", ATTRS\{idProduct\}=="([0-9a-fA-F]+)".*?TAG\+="([a-zA-Z0-9_]+)""#,
            )
            .unwrap();
            fs::read_to_string(constants::UDEV_RULES.as_str())
                .map_err(|e| error!(target: LOG, "Couldn't read {}: {e}", *constants::UDEV_RULES))
                .unwrap_or_default()
                .lines()
                .filter_map(|line| {
                    let line = line.replace(", TAG+=\"uaccess\"", "");
                    let cap = re.captures(&line)?;
                    (&cap[1] == "0b05").then(|| UsbIdentifier {
                        id_vendor: cap[1].to_string(),
                        id_product: cap[2].to_string(),
                        name: cap[3].replace('_', " "),
                    })
                })
                .collect::<Vec<_>>()
        });
        debug!(target: LOG, "Found {} compatible devices", compatible_devices.len());

        let effects = Effect::all();
        let current_effect = effects.len() - 1;
        Self {
            compatible_devices,
            effects,
            state: Mutex::new(State {
                process: None,
                conn: None,
                devices: Arc::new(vec![]),
                current_effect,
            }),
        }
    }

    pub fn compatible_devices(&self) -> &[UsbIdentifier] {
        &self.compatible_devices
    }

    pub fn available_effects(&self) -> Vec<String> {
        self.effects.iter().map(|e| e.name.to_string()).collect()
    }

    pub fn start(&self) {
        info!(target: LOG, "Starting OpenRgbClient");
        tabbed(|| {
            if ASUSCTL.available() {
                ASUSCTL.turn_off_aura();
            }
            let mut state = self.state.lock().unwrap();
            let port = free_port();
            info!(target: LOG, "Starting OpenRGB server");
            tabbed(|| {
                let log_path = format!("{}/{}.log", *constants::LOG_DIR, constants::LOG_ORGB_FILE_NAME);
                let _ = fs::remove_file(&log_path);
                let log_file = OpenOptions::new().create(true).append(true).open(&log_path);
                let mut cmd = Command::new(constants::ORGB_PATH.as_str());
                cmd.args(["--server-host", "localhost", "--server-port", &port.to_string(), "-v"])
                    .env("LD_LIBRARY_PATH", "")
                    .stdin(Stdio::null());
                // Kill OpenRGB if this process dies, so it doesn't keep the devices busy
                unsafe {
                    cmd.pre_exec(|| {
                        libc::prctl(libc::PR_SET_PDEATHSIG, libc::SIGKILL);
                        Ok(())
                    });
                }
                if let Ok(f) = log_file {
                    if let Ok(f2) = f.try_clone() {
                        cmd.stderr(f2);
                    }
                    cmd.stdout(f);
                }
                // The parent death signal fires when the spawning *thread* exits, so OpenRGB is launched
                // from a thread that lives as long as it, waiting for it
                let (tx, rx) = channel();
                let runner = thread::spawn(move || match cmd.spawn() {
                    Ok(mut child) => {
                        let _ = tx.send(Ok(child.id()));
                        let status = child.wait().map(|s| s.to_string()).unwrap_or_default();
                        info!(target: LOG, "OpenRGB server finished with {status}");
                    }
                    Err(e) => {
                        let _ = tx.send(Err(e));
                    }
                });
                match rx.recv().unwrap_or_else(|_| Err(std::io::Error::other("runner thread died"))) {
                    Ok(pid) => {
                        debug!(target: LOG, "Launched with PID {pid}");
                        state.process = Some((pid, runner));
                        let mut waited = 0;
                        while !port_in_use(port) && waited < 30_000 {
                            sleep_ms(50);
                            waited += 50;
                        }
                        // The network server starts before device detection ends: wait for it so
                        // the device list is complete
                        wait_for_detection(&log_path);
                        info!(target: LOG, "OpenRgb server ready");
                    }
                    Err(e) => error!(target: LOG, "Couldn't launch {}: {e}", *constants::ORGB_PATH),
                }
            });

            info!(target: LOG, "Connecting to server");
            let conn = tabbed(|| {
                let mut attempts = 0;
                loop {
                    match OrgbConnection::connect("localhost", port, constants::APP_NAME) {
                        Ok(c) => {
                            info!(target: LOG, "Connected");
                            break Some(Arc::new(c));
                        }
                        Err(e) if attempts < 20 => {
                            debug!(target: LOG, "Connection failed ({e}), retrying");
                            attempts += 1;
                            sleep_ms(250);
                        }
                        Err(e) => {
                            error!(target: LOG, "Couldn't connect to OpenRGB server: {e}");
                            break None;
                        }
                    }
                }
            });

            info!(target: LOG, "Getting available devices");
            let devices = tabbed(|| {
                let Some(conn) = &conn else {
                    return vec![];
                };
                match request_devices_stable(conn) {
                    Ok(devs) => devs
                        .into_iter()
                        .map(|dev| {
                            if let Some(mode) = dev.modes.iter().find(|m| m.name == "Direct") {
                                info!(target: LOG, "{}", dev.name);
                                let _ = conn.change_mode(&dev, mode);
                                let _ = conn.set_device_color(&dev, Color::BLACK);
                            }
                            Arc::new(dev)
                        })
                        .collect(),
                    Err(e) => {
                        error!(target: LOG, "Error getting devices: {e}");
                        vec![]
                    }
                }
            });
            state.conn = conn;
            state.devices = Arc::new(devices);
        });
    }

    pub fn stop(&self) {
        info!(target: LOG, "Stopping OpenRgbClient");
        tabbed(|| {
            for effect in &self.effects {
                effect.stop();
            }
            let mut state = self.state.lock().unwrap();
            if let Some(conn) = state.conn.take() {
                for dev in state.devices.iter() {
                    let _ = conn.set_device_color(dev, Color::BLACK);
                }
                conn.disconnect();
            }
            sleep_ms(100);
            info!(target: LOG, "Stopping OpenRGB server");
            if let Some((pid, runner)) = state.process.take() {
                unsafe { libc::kill(pid as i32, libc::SIGKILL) };
                let _ = runner.join();
            }
            info!(target: LOG, "OpenRGB server killed");
        });
    }

    pub fn apply_effect(&self, name: &str, brightness: RgbBrightness, color: Option<&str>) {
        for effect in &self.effects {
            effect.stop();
        }
        let mut state = self.state.lock().unwrap();
        let Some(conn) = state.conn.clone() else {
            return;
        };
        if let Some((idx, effect)) = self.effects.iter().enumerate().find(|(_, e)| e.name == name) {
            if let Some(c) = color.filter(|_| effect.supports_color()) {
                effect.set_color(c);
            }
            effect.start(conn, state.devices.clone(), brightness);
            state.current_effect = idx;
        }
    }

    pub fn disable_device(&self, name: &str) {
        let state = self.state.lock().unwrap();
        if let Some(dev) = state.devices.iter().find(|d| d.name == name) {
            dev.enabled.store(false, std::sync::atomic::Ordering::SeqCst);
        }
    }

    pub fn color(&self) -> Option<String> {
        let idx = self.state.lock().unwrap().current_effect;
        self.effects.get(idx).and_then(|e| e.color())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Launches the bundled OpenRGB on a free port and lists the detected devices, without changing
    /// any mode or color. Run it with the app closed: `cargo test --lib -- --ignored --nocapture lists_devices`
    #[test]
    #[ignore]
    fn lists_devices_from_bundled_openrgb() {
        let port = free_port();
        let log_path = std::env::temp_dir().join(format!("orgb-test-{port}.log")).display().to_string();
        let log = OpenOptions::new().create(true).write(true).truncate(true).open(&log_path).unwrap();
        let mut child = Command::new(constants::ORGB_PATH.as_str())
            .args(["--server-host", "localhost", "--server-port", &port.to_string(), "-v"])
            .env("LD_LIBRARY_PATH", "")
            .stdout(log.try_clone().unwrap())
            .stderr(log)
            .spawn()
            .expect("bundled OpenRGB not found, run `make run` once to build it");
        while !port_in_use(port) {
            sleep_ms(50);
        }
        wait_for_detection(&log_path);
        let conn = OrgbConnection::connect("localhost", port, "RogPerfTuner test").unwrap();
        let devices = request_devices_stable(&conn).unwrap();
        conn.disconnect();
        let _ = child.kill();
        let _ = child.wait();
        let _ = fs::remove_file(&log_path);
        for dev in &devices {
            let direct = dev.modes.iter().any(|m| m.name == "Direct");
            println!("{} ({} leds, direct mode: {direct})", dev.name, dev.leds.len());
        }
        assert!(!devices.is_empty());
    }
}
