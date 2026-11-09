//! Color fading in and out.

use std::f64::consts::PI;

use std::sync::Arc;

use crate::clients::tcp::open_rgb::{effects::EffectCtx, protocol::Device};

pub fn run(ctx: &EffectCtx, devices: &[Arc<Device>]) {
    let (total, pause, freq) = (4.0, 1.0, 0.05);
    let steps = ((total - pause) / freq) as usize;
    let sin_steps: Vec<f64> = (0..steps).map(|o| (PI * (o as f64 / (steps - 1) as f64)).sin()).collect();
    let mut offset = 0;
    while ctx.running() {
        let color = ctx.color().scale(sin_steps[offset]);
        for dev in devices.iter().filter(|d| d.enabled()) {
            ctx.set_colors(dev, &vec![color; dev.leds.len()]);
        }
        offset = (offset + 1) % steps;
        ctx.sleep(if offset > 0 { freq } else { pause });
    }
}
