//! Aura lighting service.

use std::{
    sync::{LazyLock, Mutex},
    time::Instant,
};

use log::info;

use crate::{
    clients::tcp::open_rgb::open_rgb_client::OPENRGB,
    models::{hardware::RgbBrightness, settings::EffectConfig},
    services::configuration,
    utils::{
        elapsed_secs,
        event_bus::{self, Event},
        logger::tabbed,
    },
};

const LOG: &str = "OpenRgbService";

struct State {
    effect: String,
    brightness: RgbBrightness,
    color: Option<String>,
}

pub struct OpenRgbService {
    state: Mutex<State>,
    action: Mutex<()>,
}

static SERVICE: LazyLock<OpenRgbService> = LazyLock::new(|| {
    info!(target: LOG, "Initializing OpenRgbService");
    tabbed(|| {
        OPENRGB.start();
        let service = OpenRgbService {
            state: Mutex::new(State {
                effect: "Static".into(),
                brightness: RgbBrightness::Max,
                color: None,
            }),
            action: Mutex::new(()),
        };
        service.restore_aura();
        service
    })
});

pub fn get() -> &'static OpenRgbService {
    &SERVICE
}

impl OpenRgbService {
    pub fn start(&'static self) {
        event_bus::subscribe(|e| match e {
            Event::OnBattery(on_bat) => {
                let s = get().state.lock().unwrap();
                let brightness = if *on_bat { RgbBrightness::Off } else { s.brightness };
                OPENRGB.apply_effect(&s.effect, brightness, s.color.as_deref());
            }
            Event::UsbAdded => get().reload(),
            Event::UsbRemoved(devs) => {
                for dev in devs {
                    info!(target: LOG, "Disabling {}", dev.name);
                    OPENRGB.disable_device(&dev.name);
                }
            }
            Event::ApplicationShutdown => OPENRGB.stop(),
            _ => {}
        });
    }

    fn color_for(effect: &str) -> Option<String> {
        configuration::read(|c| c.aura.config.get(effect).map(|e| e.color.clone()))
    }

    pub fn restore_aura(&self) {
        let (effect, brightness) = configuration::read(|c| {
            (c.aura.last_effect.clone().unwrap_or_else(|| "Static".into()), c.aura.brightness)
        });
        {
            let mut s = self.state.lock().unwrap();
            s.effect = effect.clone();
            s.brightness = brightness;
            s.color = Self::color_for(&effect);
        }
        self.apply_aura(false);
        let mut s = self.state.lock().unwrap();
        if s.color.is_none() {
            s.color = OPENRGB.color();
        }
        event_bus::emit(Event::RgbEffect(s.effect.clone()));
        event_bus::emit(Event::RgbBrightness(s.brightness));
        event_bus::emit(Event::RgbColor(s.color.clone()));
    }

    pub fn available_effects(&self) -> Vec<String> {
        let mut all = OPENRGB.available_effects();
        all.sort();
        all
    }

    pub fn current_effect(&self) -> String {
        self.state.lock().unwrap().effect.clone()
    }

    pub fn current_brightness(&self) -> RgbBrightness {
        self.state.lock().unwrap().brightness
    }

    pub fn color(&self) -> Option<String> {
        self.state.lock().unwrap().color.clone()
    }

    pub fn set_brightness(&self, brightness: RgbBrightness) {
        let _guard = self.action.lock().unwrap();
        if self.current_brightness() != brightness {
            self.state.lock().unwrap().brightness = brightness;
            self.apply_aura(false);
            event_bus::emit(Event::RgbBrightness(brightness));
        }
    }

    pub fn set_effect(&self, effect: &str, temporal: bool) {
        let _guard = self.action.lock().unwrap();
        if self.current_effect() != effect {
            {
                let mut s = self.state.lock().unwrap();
                s.effect = effect.to_string();
                s.color = Self::color_for(effect);
            }
            self.apply_aura(temporal);
            {
                let mut s = self.state.lock().unwrap();
                if s.color.is_none() {
                    s.color = OPENRGB.color();
                }
            }
            event_bus::emit(Event::RgbEffect(effect.to_string()));
            event_bus::emit(Event::RgbColor(OPENRGB.color()));
        }
    }

    pub fn set_color(&self, color: &str) {
        let _guard = self.action.lock().unwrap();
        let supports = self.state.lock().unwrap().color.is_some();
        if supports {
            self.state.lock().unwrap().color = Some(color.to_string());
            self.apply_aura(false);
            event_bus::emit(Event::RgbColor(OPENRGB.color()));
        }
    }

    fn reload(&self) {
        let t0 = Instant::now();
        info!(target: LOG, "Reloading OpenRGB server");
        tabbed(|| {
            OPENRGB.stop();
            OPENRGB.start();
            self.apply_aura(false);
        });
        info!(target: LOG, "Reloaded after {} seconds", elapsed_secs(t0));
    }

    fn apply_aura(&self, temporal: bool) {
        info!(target: LOG, "Applying aura settings");
        let t0 = Instant::now();
        tabbed(|| {
            let (effect, brightness, color) = {
                let s = self.state.lock().unwrap();
                (s.effect.clone(), s.brightness, s.color.clone())
            };
            OPENRGB.apply_effect(&effect, brightness, color.as_deref());
            if !temporal {
                configuration::update(|c| {
                    c.aura.brightness = brightness;
                    c.aura.last_effect = Some(effect.clone());
                    if let Some(color) = &color {
                        c.aura.config.insert(effect.clone(), EffectConfig { color: color.clone() });
                    }
                });
            }
        });
        info!(target: LOG, "Aura applied after {} seconds", elapsed_secs(t0));
    }

    pub fn increase_brightness(&self) -> RgbBrightness {
        let next = self.current_brightness().next();
        self.set_brightness(next);
        next
    }

    pub fn decrease_brightness(&self) -> RgbBrightness {
        let next = self.current_brightness().previous();
        self.set_brightness(next);
        next
    }

    pub fn next_effect(&self) -> String {
        let list = self.available_effects();
        let current = self.current_effect();
        let idx = list.iter().position(|e| *e == current).map(|i| (i + 1) % list.len()).unwrap_or(0);
        let next = list[idx].clone();
        self.set_effect(&next, false);
        next
    }
}
