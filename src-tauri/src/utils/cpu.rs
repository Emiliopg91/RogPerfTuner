//! CPU usage sampling.

use std::fs;

use crate::utils::sleep_ms;

fn read_cpu() -> (u64, u64) {
    let line = fs::read_to_string("/proc/stat")
        .ok()
        .and_then(|c| c.lines().next().map(|l| l.to_string()))
        .unwrap_or_default();
    let vals: Vec<u64> = line.split_whitespace().skip(1).take(7).filter_map(|v| v.parse().ok()).collect();
    if vals.len() < 7 {
        return (0, 0);
    }
    let total: u64 = vals.iter().sum();
    let active = total - vals[3] - vals[4];
    (active, total)
}

/// CPU usage ratio (0..1) sampled over `interval_ms`.
pub fn cpu_use_rate(interval_ms: u64) -> f64 {
    let (a1, t1) = read_cpu();
    sleep_ms(interval_ms);
    let (a2, t2) = read_cpu();
    if t2 <= t1 {
        return 0.0;
    }
    (a2.saturating_sub(a1)) as f64 / (t2 - t1) as f64
}
