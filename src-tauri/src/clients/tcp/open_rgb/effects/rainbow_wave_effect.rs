//! Rainbow moving across the zones.

use std::sync::Arc;

use crate::clients::tcp::open_rgb::{effects::EffectCtx, protocol::{Color, Device, ZONE_TYPE_MATRIX}};

pub fn run(ctx: &EffectCtx, devices: &[Arc<Device>]) {
    let longest = devices
        .iter()
        .flat_map(|d| d.zones.iter())
        .map(|z| if z.kind == ZONE_TYPE_MATRIX { z.matrix_width } else { z.leds_count } as usize)
        .max()
        .unwrap_or(1)
        .max(1);
    let size = 3 * longest;
    let inc = (360.0 / size as f64) as i32;
    let mut rainbow: Vec<i32> = (0..size).map(|idx| (size - idx) as i32 * inc).collect();

    while ctx.running() {
        for dev in devices.iter().filter(|d| d.enabled()) {
            let mut colors = vec![Color::BLACK; dev.leds.len()];
            let mut offset = 0usize;
            for zone in &dev.zones {
                if zone.kind == ZONE_TYPE_MATRIX {
                    for r in 0..zone.matrix_height as usize {
                        for c in 0..zone.matrix_width as usize {
                            let v = zone.matrix_values[r * zone.matrix_width as usize + c] as usize;
                            if v < colors.len() && offset + v < colors.len() {
                                let idx = ((size as f64 * (c as f64 / zone.matrix_width as f64)).round() as usize).min(size - 1);
                                colors[offset + v] = Color::from_hsv(rainbow[idx] as f64, 1.0, 1.0);
                            }
                        }
                    }
                } else {
                    for l in 0..zone.leds_count as usize {
                        let idx = ((size as f64 * (l as f64 / zone.leds_count as f64)).floor() as usize).min(size - 1);
                        if offset + l < colors.len() {
                            colors[offset + l] = Color::from_hsv(rainbow[idx] as f64, 1.0, 1.0);
                        }
                    }
                }
                offset += zone.leds_count as usize;
            }
            ctx.set_colors(dev, &colors);
        }
        ctx.sleep(4.0 / size as f64);
        rainbow.insert(0, (rainbow[0] + inc) % 360);
        rainbow.pop();
    }
}
