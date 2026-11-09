//! Random numbers (SplitMix64, enough for lighting effects and ids).

use std::{
    sync::atomic::{AtomicU64, Ordering},
    time::{SystemTime, UNIX_EPOCH},
};

static SEED: AtomicU64 = AtomicU64::new(0);

pub fn random_u64() -> u64 {
    if SEED.load(Ordering::Relaxed) == 0 {
        let now = SystemTime::now().duration_since(UNIX_EPOCH).unwrap_or_default().as_nanos() as u64;
        let _ = SEED.compare_exchange(0, now | 1, Ordering::SeqCst, Ordering::SeqCst);
    }
    let mut z = SEED.fetch_add(0x9E37_79B9_7F4A_7C15, Ordering::Relaxed).wrapping_add(0x9E37_79B9_7F4A_7C15);
    z = (z ^ (z >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
    z ^ (z >> 31)
}

/// Random integer in `[min, max]` (inclusive).
pub fn random_range(min: i64, max: i64) -> i64 {
    if max <= min {
        return min;
    }
    min + (random_u64() % ((max - min + 1) as u64)) as i64
}

pub fn random_f64() -> f64 {
    (random_u64() >> 11) as f64 / (1u64 << 53) as f64
}

pub fn uuid_v4() -> String {
    let a = random_u64();
    let b = random_u64();
    format!(
        "{:08x}-{:04x}-4{:03x}-{:04x}-{:012x}",
        (a >> 32) as u32,
        (a >> 16) as u16,
        (a as u16) & 0x0fff,
        ((b >> 48) as u16 & 0x3fff) | 0x8000,
        b & 0xffff_ffff_ffff
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn uuid_has_v4_shape() {
        let id = uuid_v4();
        assert_eq!(id.len(), 36);
        assert_eq!(&id[14..15], "4");
    }
}
