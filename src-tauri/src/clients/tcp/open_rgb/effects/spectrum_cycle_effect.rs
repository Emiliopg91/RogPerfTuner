//! Whole device cycling through the hue wheel.

use std::sync::Arc;

use crate::clients::tcp::open_rgb::{effects::EffectCtx, protocol::{Color, Device}};

pub fn run(ctx: &EffectCtx, devices: &[Arc<Device>]) {
    let mut offset = 0;
    while ctx.running() {
        let color = Color::from_hsv(offset as f64, 1.0, 1.0);
        for dev in devices.iter().filter(|d| d.enabled()) {
            ctx.set_colors(dev, &vec![color; dev.leds.len()]);
        }
        offset = (offset + 1) % 360;
        ctx.sleep(0.02);
    }
}
