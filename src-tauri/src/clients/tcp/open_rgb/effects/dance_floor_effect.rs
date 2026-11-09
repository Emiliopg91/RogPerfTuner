//! Random colors on every LED.

use crate::utils::random::random_range;
use std::sync::Arc;

use crate::clients::tcp::open_rgb::{effects::EffectCtx, protocol::{Color, Device}};

pub fn run(ctx: &EffectCtx, devices: &[Arc<Device>]) {
    while ctx.running() {
        for dev in devices.iter().filter(|d| d.enabled()) {
            let colors: Vec<Color> = (0..dev.leds.len())
                .map(|_| Color::from_hsv(random_range(0, 359) as f64, random_range(80, 100) as f64 / 100.0, 1.0))
                .collect();
            ctx.set_colors(dev, &colors);
        }
        ctx.sleep(0.5);
    }
}
