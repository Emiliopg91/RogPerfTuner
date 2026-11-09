//! LEDs randomly lighting up in white, red, cyan or off.

use std::thread;

use crate::{clients::tcp::open_rgb::effects::shuffle, utils::random::random_range};
use std::sync::Arc;

use crate::clients::tcp::open_rgb::{effects::EffectCtx, protocol::{Color, Device}};

pub fn run(ctx: &EffectCtx, devices: &[Arc<Device>]) {
    let palette = [Color::BLACK, Color::BLACK, Color::WHITE, Color::RED, Color::new(0, 255, 255)];
    thread::scope(|s| {
        for dev in devices {
            s.spawn(move || {
                let len = dev.leds.len();
                if len == 0 {
                    return;
                }
                let mut leds = vec![Color::BLACK; len];
                let mut buffer: Vec<(usize, Color)> = vec![];
                while dev.enabled() && ctx.running() {
                    if buffer.is_empty() {
                        buffer = (0..len)
                            .map(|i| (i, palette[random_range(0, palette.len() as i64 - 1) as usize]))
                            .collect();
                        shuffle(&mut buffer);
                    }
                    let (idx, color) = buffer.remove(0);
                    leds[idx] = color;
                    ctx.set_colors(dev, &leds);
                    let naps = 4.0;
                    for _ in 0..4 {
                        if dev.enabled() && ctx.running() {
                            let nap = 2500.0 / len as f64 / (random_range(6, 10) as f64 / 10.0) / naps;
                            ctx.sleep(nap / 1000.0);
                        }
                    }
                }
            });
        }
    });
}
