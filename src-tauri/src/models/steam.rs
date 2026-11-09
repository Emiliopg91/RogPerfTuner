use std::collections::HashMap;

use serde::{Deserialize, Serialize};

use crate::models::lenient;

/// Launch configuration sent to the `run` wrapper through the unix socket
#[derive(Debug, Clone, Default, Serialize, Deserialize)]
#[serde(default)]
pub struct SteamGameConfig {
    #[serde(deserialize_with = "lenient::string_map")]
    pub environment: HashMap<String, String>,
    #[serde(deserialize_with = "lenient::string_vec")]
    pub wrappers: Vec<String>,
    #[serde(deserialize_with = "lenient::string")]
    pub parameters: String,
}

fn default_launch_opts() -> String {
    "%command%".to_string()
}

fn default_true() -> bool {
    true
}

/// App details reported by the Decky plugin
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SteamGameDetails {
    #[serde(default)]
    pub appid: u32,
    #[serde(default, deserialize_with = "lenient::string")]
    pub name: String,
    #[serde(default = "default_true")]
    pub is_steam_app: bool,
    #[serde(default = "default_launch_opts", deserialize_with = "lenient::string")]
    pub launch_opts: String,
    #[serde(default, deserialize_with = "lenient::string")]
    pub icon_hash: String,
    #[serde(default, deserialize_with = "lenient::string")]
    pub compat_tool: String,
    #[serde(default)]
    pub is_shortcut: bool,
}

/// Map of app id -> details, as returned by `get_apps_details`
pub type SteamGameDetailsMap = HashMap<String, SteamGameDetails>;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SemanticVersion {
    pub major: u32,
    pub minor: u32,
    pub patch: u32,
    pub suffix: String,
}

impl SemanticVersion {
    pub fn parse(version: &str) -> Self {
        let v = version.trim();
        let mut parts = v.splitn(3, '.');
        let major = parts.next().and_then(|p| p.parse().ok()).unwrap_or(0);
        let minor = parts.next().and_then(|p| p.parse().ok()).unwrap_or(0);
        let (patch, suffix) = match parts.next() {
            Some(rest) => match rest.find(['-', '.']) {
                Some(pos) => (rest[..pos].parse().unwrap_or(0), rest[pos + 1..].to_string()),
                None => (rest.parse().unwrap_or(0), String::new()),
            },
            None => (0, String::new()),
        };
        Self { major, minor, patch, suffix }
    }

    fn numeric_suffix(&self) -> Option<u64> {
        (!self.suffix.is_empty() && self.suffix.chars().all(|c| c.is_ascii_digit()))
            .then(|| self.suffix.parse().ok())
            .flatten()
    }
}

impl PartialOrd for SemanticVersion {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        use std::cmp::Ordering::*;
        let base = (self.major, self.minor, self.patch).cmp(&(other.major, other.minor, other.patch));
        if base != Equal {
            return Some(base);
        }
        Some(match (self.numeric_suffix(), other.numeric_suffix()) {
            (Some(a), Some(b)) => a.cmp(&b),
            _ if self.suffix.is_empty() && !other.suffix.is_empty() => Less,
            _ if !self.suffix.is_empty() && other.suffix.is_empty() => Greater,
            _ => Equal,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::SemanticVersion;

    #[test]
    fn version_comparison() {
        assert!(SemanticVersion::parse("1.2.8") > SemanticVersion::parse("1.2.7"));
        assert!(SemanticVersion::parse("1.3.0") > SemanticVersion::parse("1.2.9"));
        // Non numeric suffixes are not comparable
        assert!(SemanticVersion::parse("1.2.8.r3") <= SemanticVersion::parse("1.2.8.r2"));
        assert!(SemanticVersion::parse("1.2.8.5") > SemanticVersion::parse("1.2.8.2"));
    }
}
