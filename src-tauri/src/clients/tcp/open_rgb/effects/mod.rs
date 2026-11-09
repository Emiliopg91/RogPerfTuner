//! Lighting effects. Holds what every effect shares: the runner thread, the drawing context and helpers.
//! Each effect has its own file exposing `run(ctx, devices)`.

pub mod breathing_effect;
pub mod dance_floor_effect;
pub mod digital_rain_effect;
pub mod drops_effect;
pub mod gaming_effect;
pub mod rainbow_wave_effect;
pub mod spectrum_cycle_effect;
pub mod starry_night_effect;
pub mod static_effect;

use std::{
    collections::HashMap,
    sync::{
        Arc, Mutex,
        atomic::{AtomicBool, Ordering},
    },
    thread::{self, JoinHandle},
};

use log::info;

use crate::{
    clients::tcp::open_rgb::protocol::{Color, Device, OrgbConnection},
    models::hardware::RgbBrightness,
    utils::{random::random_range, sleep_ms},
};

pub type Devices = Arc<Vec<Arc<Device>>>;

/// What an effect thread gets to work with
pub struct EffectCtx {
    conn: Arc<OrgbConnection>,
    running: Arc<AtomicBool>,
    brightness: f64,
    color: Option<Color>,
    /// Last frame sent to each device (by index); also serializes the writes
    last_frames: Mutex<HashMap<u32, Vec<Color>>>,
}

impl EffectCtx {
    pub fn running(&self) -> bool {
        self.running.load(Ordering::SeqCst)
    }

    /// Sleeps in slices of 100ms so a stop request is honored quickly.
    pub fn sleep(&self, seconds: f64) {
        let mut remaining = seconds * 1000.0;
        while self.running() && remaining > 0.0 {
            let nap = remaining.min(100.0);
            sleep_ms(nap as u64);
            remaining -= nap;
        }
    }

    pub fn color(&self) -> Color {
        self.color.unwrap_or(Color::RED)
    }

    pub fn set_colors(&self, dev: &Device, colors: &[Color]) {
        if dev.enabled() && self.running() {
            let mut last_frames = self.last_frames.lock().unwrap();
            if self.running() {
                let dimmed: Vec<Color> = colors.iter().map(|c| c.scale(self.brightness)).collect();
                // Identical frames (static effects, idle digital rain...) aren't sent again
                if last_frames.get(&dev.idx) != Some(&dimmed) && self.conn.set_device_colors(dev, &dimmed).is_ok() {
                    last_frames.insert(dev.idx, dimmed);
                }
            }
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EffectKind {
    Breathing,
    DanceFloor,
    DigitalRain,
    Drops,
    Gaming,
    RainbowWave,
    SpectrumCycle,
    StarryNight,
    Static,
}

pub struct Effect {
    pub name: &'static str,
    kind: EffectKind,
    color: Mutex<Option<Color>>,
    running: Arc<AtomicBool>,
    thread: Mutex<Option<JoinHandle<()>>>,
}

impl Effect {
    fn new(name: &'static str, kind: EffectKind, color: Option<Color>) -> Self {
        Self {
            name,
            kind,
            color: Mutex::new(color),
            running: Arc::new(AtomicBool::new(false)),
            thread: Mutex::new(None),
        }
    }

    /// Every available effect
    pub fn all() -> Vec<Effect> {
        vec![
            Effect::new("Breathing", EffectKind::Breathing, Some(Color::RED)),
            Effect::new("Dance floor", EffectKind::DanceFloor, None),
            Effect::new("Digital rain", EffectKind::DigitalRain, Some(Color::GREEN)),
            Effect::new("Drops", EffectKind::Drops, None),
            Effect::new("Gaming", EffectKind::Gaming, None),
            Effect::new("Rainbow wave", EffectKind::RainbowWave, None),
            Effect::new("Spectrum cycle", EffectKind::SpectrumCycle, None),
            Effect::new("Starry night", EffectKind::StarryNight, None),
            Effect::new("Static", EffectKind::Static, Some(Color::RED)),
        ]
    }

    fn log_name(&self) -> String {
        let name: String = self.name.split(' ').map(crate::utils::capitalize).collect();
        format!("{name}Effect")
    }

    pub fn supports_color(&self) -> bool {
        self.color.lock().unwrap().is_some()
    }

    pub fn set_color(&self, hex: &str) {
        let mut c = self.color.lock().unwrap();
        if c.is_some() {
            *c = Some(Color::from_hex(hex));
        }
    }

    pub fn color(&self) -> Option<String> {
        self.color.lock().unwrap().map(|c| c.to_hex())
    }

    pub fn start(&self, conn: Arc<OrgbConnection>, devices: Devices, brightness: RgbBrightness) {
        self.stop();
        let log = self.log_name();
        if brightness == RgbBrightness::Off {
            info!(target: log.as_str(), "Turning off RGB");
            for dev in devices.iter() {
                let _ = conn.set_device_color(dev, Color::BLACK);
            }
            return;
        }
        let color = *self.color.lock().unwrap();
        match color {
            Some(c) => info!(target: log.as_str(), "Starting effect {} with {} brightness and color {}", self.name, brightness.name().to_lowercase(), c.to_hex().to_uppercase()),
            None => info!(target: log.as_str(), "Starting effect {} with {} brightness", self.name, brightness.name().to_lowercase()),
        }
        self.running.store(true, Ordering::SeqCst);
        let ctx = EffectCtx {
            conn,
            running: self.running.clone(),
            brightness: brightness.value() as f64 / 100.0,
            color,
            last_frames: Mutex::new(HashMap::new()),
        };
        let kind = self.kind;
        let handle = thread::spawn(move || {
            match kind {
                EffectKind::Breathing => breathing_effect::run(&ctx, &devices),
                EffectKind::DanceFloor => dance_floor_effect::run(&ctx, &devices),
                EffectKind::DigitalRain => digital_rain_effect::run(&ctx, &devices),
                EffectKind::Drops => drops_effect::run(&ctx, &devices),
                EffectKind::Gaming => gaming_effect::run(&ctx, &devices),
                EffectKind::RainbowWave => rainbow_wave_effect::run(&ctx, &devices),
                EffectKind::SpectrumCycle => spectrum_cycle_effect::run(&ctx, &devices),
                EffectKind::StarryNight => starry_night_effect::run(&ctx, &devices),
                EffectKind::Static => static_effect::run(&ctx, &devices),
            }
            info!(target: log.as_str(), "Effect finished");
        });
        *self.thread.lock().unwrap() = Some(handle);
    }

    pub fn stop(&self) {
        if self.running.swap(false, Ordering::SeqCst) {
            info!(target: self.log_name().as_str(), "Stopping effect");
        }
        if let Some(handle) = self.thread.lock().unwrap().take() {
            let _ = handle.join();
        }
    }
}

/// Fisher-Yates shuffle
pub(crate) fn shuffle<T>(v: &mut [T]) {
    for i in (1..v.len()).rev() {
        let j = random_range(0, i as i64) as usize;
        v.swap(i, j);
    }
}
