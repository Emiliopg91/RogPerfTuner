//! Matrix-like rain whose density follows the CPU usage.

use std::{
    f64::consts::PI,
    sync::atomic::{AtomicU64, Ordering},
    thread,
};

use crate::utils::{cpu::cpu_use_rate, random::random_range};
use std::sync::Arc;

use crate::clients::tcp::open_rgb::{effects::EffectCtx, protocol::{Color, DEVICE_TYPE_MOUSE, Device, ZONE_TYPE_MATRIX}};

const INVALID_LED: u32 = u32::MAX;

#[derive(Clone, Copy)]
struct LedStatus {
    pos_idx: u32,
    max_val: i32,
    cur_val: i32,
}

impl LedStatus {
    fn new(pos_idx: u32) -> Self {
        Self { pos_idx, max_val: 20, cur_val: 0 }
    }
}

fn dev_to_matrix(dev: &Device) -> Vec<Vec<LedStatus>> {
    let mut mat: Vec<Vec<LedStatus>> = vec![];
    let mut offset = 0u32;
    let mut last_leds = 0u32;
    for zone in &dev.zones {
        if zone.kind == ZONE_TYPE_MATRIX {
            for r in 0..zone.matrix_height {
                let row = (0..zone.matrix_width)
                    .map(|c| {
                        let v = zone.matrix_values[(r * zone.matrix_width + c) as usize];
                        LedStatus::new(if v != INVALID_LED { offset + v } else { INVALID_LED })
                    })
                    .collect();
                mat.push(row);
            }
        } else {
            mat.push((0..zone.leds_count).map(|l| LedStatus::new(offset + l)).collect());
        }
        offset += zone.leds_count;
        last_leds = zone.leds_count;
    }
    if dev.kind == DEVICE_TYPE_MOUSE {
        offset -= last_leds;
        for led in mat.iter_mut().flatten().filter(|l| l.pos_idx != INVALID_LED) {
            led.pos_idx = offset.wrapping_sub(led.pos_idx);
        }
    }
    mat
}

fn decrement_matrix(m: &mut [Vec<LedStatus>]) {
    for r in (0..m.len()).rev() {
        if r == 0 {
            for led in m[0].iter_mut() {
                led.cur_val -= led.cur_val.signum();
            }
            continue;
        }
        let (above, below) = m.split_at_mut(r);
        for (dst, src) in below[0].iter_mut().zip(above[r - 1].iter()) {
            let pos = dst.pos_idx;
            *dst = *src;
            dst.pos_idx = pos;
        }
    }
}

fn column_available(m: &[Vec<LedStatus>], column: usize) -> bool {
    if m.is_empty() || m[0].is_empty() {
        return false;
    }
    (0..m.len().min(3)).all(|r| column < m[r].len() && m[r][column].cur_val == 0)
}

pub fn run(ctx: &EffectCtx, devices: &[Arc<Device>]) {
    let max_count = 15i32;
    let nap = 0.07;
    let sin_array: Vec<f64> = (0..(2 * max_count / 3) as usize)
        .map(|i| ((i as f64 / (max_count + 2) as f64) * PI / 2.0).sin().powi(2))
        .collect();
    let cpu = AtomicU64::new(0.0f64.to_bits());
    let cpu_val = || f64::from_bits(cpu.load(Ordering::Relaxed));

    thread::scope(|s| {
        s.spawn(|| {
            while ctx.running() {
                cpu.store(cpu_use_rate(20).max(0.01).to_bits(), Ordering::Relaxed);
                ctx.sleep(2.0 * nap);
            }
        });
        for dev in devices {
            let sin_array = &sin_array;
            let cpu_val = &cpu_val;
            s.spawn(move || {
                let mut status = dev_to_matrix(dev);
                let size = dev.leds.len();
                ctx.set_colors(dev, &vec![Color::BLACK; size]);
                ctx.sleep(random_range(0, 500) as f64 / 1000.0);
                if status.is_empty() {
                    return;
                }
                while dev.enabled() && ctx.running() {
                    let cpu = cpu_val();
                    decrement_matrix(&mut status);

                    let free: Vec<usize> = (0..status[0].len()).filter(|c| column_available(&status, *c)).collect();
                    if !free.is_empty() {
                        let cols = status[0].len();
                        let allowed = ((cols as f64 * cpu).ceil() as usize).max(1);
                        if allowed > cols - free.len() {
                            let next = free[random_range(0, free.len() as i64 - 1) as usize];
                            let max_val = (max_count as f64 * (1.0 - 0.25 * cpu)).round() as i32;
                            status[0][next].max_val = max_val;
                            status[0][next].cur_val = max_val;
                        }
                    }

                    let mut colors = vec![Color::BLACK; size];
                    for led in status.iter().flatten() {
                        if (led.pos_idx as usize) < size {
                            colors[led.pos_idx as usize] = if led.cur_val >= led.max_val {
                                Color::WHITE
                            } else if led.cur_val >= 2 * led.max_val / 3 {
                                ctx.color()
                            } else {
                                let idx = (led.cur_val.max(0) as usize).min(sin_array.len() - 1);
                                ctx.color().scale(sin_array[idx])
                            };
                        }
                    }
                    ctx.set_colors(dev, &colors);
                    ctx.sleep(nap * (1.0 - 0.4 * cpu));
                }
            });
        }
    });
}
