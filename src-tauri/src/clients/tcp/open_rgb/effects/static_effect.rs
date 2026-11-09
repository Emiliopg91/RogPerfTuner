//! Solid color on every LED.

use std::sync::Arc;

use crate::clients::tcp::open_rgb::{effects::EffectCtx, protocol::Device};

pub fn run(ctx: &EffectCtx, devices: &[Arc<Device>]) {
    for dev in devices {
        ctx.set_colors(dev, &vec![ctx.color(); dev.leds.len()]);
    }
    while ctx.running() {
        ctx.sleep(1.0);
    }
}
