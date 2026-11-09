//! Random stars fading out.

use std::thread;

use crate::utils::random::{random_f64, random_range};
use std::sync::Arc;

use crate::clients::tcp::open_rgb::{effects::EffectCtx, protocol::{Color, Device}};

pub fn run(ctx: &EffectCtx, devices: &[Arc<Device>]) {
    let max_steps = 30.0;
    thread::scope(|s| {
        for dev in devices {
            s.spawn(move || {
                let len = dev.leds.len();
                if len == 0 {
                    return;
                }
                let mut leds = vec![Color::BLACK; len];
                let mut steps = vec![0i32; len];
                while dev.enabled() && ctx.running() {
                    let mut new_colors = vec![Color::BLACK; len];
                    let mut active = 0;
                    for i in 0..len {
                        steps[i] = (steps[i] - 1).max(0);
                        new_colors[i] = leds[i].scale(steps[i] as f64 / max_steps);
                        if steps[i] > 0 {
                            active += 1;
                        }
                    }
                    if (active as f64 / len as f64) < 0.2 {
                        let mut on = random_range(0, len as i64 - 1) as usize;
                        while steps[on] > 0 {
                            on = random_range(0, len as i64 - 1) as usize;
                        }
                        steps[on] = 20 + random_range(0, 10) as i32;
                        let c = Color::from_hsv(random_range(0, 359) as f64, 1.0, 1.0).scale(steps[on] as f64 / max_steps);
                        leds[on] = c;
                        new_colors[on] = c;
                    }
                    ctx.set_colors(dev, &new_colors);
                    ctx.sleep(random_f64() * 0.15);
                }
            });
        }
    });
}
