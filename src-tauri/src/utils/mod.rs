//! Utilities. Holds the small helpers shared everywhere; each area has its own file.

pub mod base64;
pub mod constants;
pub mod cpu;
pub mod event_bus;
pub mod files;
pub mod logger;
pub mod net;
pub mod process;
pub mod random;
pub mod shell;
pub mod single_instance;
pub mod translator;

use std::{collections::HashMap, thread, time::{Duration, Instant}};

pub fn sleep_ms(ms: u64) {
    thread::sleep(Duration::from_millis(ms));
}

/// Formats an elapsed time as seconds with 3 decimals, like `TimeUtils::format_seconds`.
pub fn elapsed_secs(t0: Instant) -> String {
    format!("{:.3}", t0.elapsed().as_secs_f64())
}

pub fn capitalize(s: &str) -> String {
    let mut chars = s.chars();
    match chars.next() {
        Some(f) => f.to_uppercase().collect::<String>() + &chars.as_str().to_lowercase(),
        None => String::new(),
    }
}

/// Parses a `KEY=VAL KEY2=VAL2` string into a map, ignoring tokens without `=`.
pub fn parse_env(env: &str) -> HashMap<String, String> {
    env.split_whitespace()
        .filter_map(|tok| tok.split_once('=').map(|(k, v)| (k.to_string(), v.to_string())))
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn env_parsing_skips_plain_tokens() {
        let env = parse_env("A=1 foo B=x=y");
        assert_eq!(env.get("A").unwrap(), "1");
        assert_eq!(env.get("B").unwrap(), "x=y");
        assert_eq!(env.len(), 2);
    }
}
