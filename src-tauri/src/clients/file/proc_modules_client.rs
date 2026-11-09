//! Loaded kernel modules.

use std::fs;

pub fn is_module_loaded(module: &str) -> bool {
    fs::read_to_string("/proc/modules")
        .map(|c| c.lines().any(|l| l.starts_with(&format!("{module} "))))
        .unwrap_or(false)
}
