use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize, Default, PartialOrd, Ord)]
#[serde(rename_all = "lowercase")]
pub enum PerformanceProfile {
    #[default]
    Smart,
    Performance,
    Balanced,
    Quiet,
}

impl PerformanceProfile {
    pub const ALL: [PerformanceProfile; 4] = [Self::Smart, Self::Performance, Self::Balanced, Self::Quiet];

    /// Upper case name, used for translation keys and socket responses.
    pub fn name(&self) -> &'static str {
        match self {
            Self::Smart => "SMART",
            Self::Performance => "PERFORMANCE",
            Self::Balanced => "BALANCED",
            Self::Quiet => "QUIET",
        }
    }

    /// Lower case name, used as config key.
    pub fn as_str(&self) -> &'static str {
        match self {
            Self::Smart => "smart",
            Self::Performance => "performance",
            Self::Balanced => "balanced",
            Self::Quiet => "quiet",
        }
    }

    pub fn from_str(s: &str) -> Option<Self> {
        match s.to_lowercase().as_str() {
            "smart" => Some(Self::Smart),
            "performance" => Some(Self::Performance),
            "balanced" => Some(Self::Balanced),
            "quiet" | "low_power" => Some(Self::Quiet),
            _ => None,
        }
    }

    pub fn next(&self, circular: bool, include_smart: bool) -> Self {
        match self {
            Self::Smart => {
                if circular {
                    Self::Quiet
                } else {
                    Self::Smart
                }
            }
            Self::Performance => {
                if include_smart {
                    Self::Smart
                } else if circular {
                    Self::Quiet
                } else {
                    Self::Performance
                }
            }
            Self::Balanced => Self::Performance,
            Self::Quiet => Self::Balanced,
        }
    }

    pub fn previous(&self, circular: bool) -> Self {
        match self {
            Self::Performance => Self::Balanced,
            Self::Balanced => Self::Quiet,
            Self::Quiet => {
                if circular {
                    Self::Performance
                } else {
                    Self::Quiet
                }
            }
            Self::Smart => Self::Smart,
        }
    }

    pub fn platform_profile(&self) -> PlatformProfile {
        match self {
            Self::Quiet => PlatformProfile::Quiet,
            Self::Balanced => PlatformProfile::Balanced,
            _ => PlatformProfile::Performance,
        }
    }

    pub fn power_profile(&self) -> PowerProfile {
        match self {
            Self::Quiet => PowerProfile::Quiet,
            Self::Balanced => PowerProfile::Balanced,
            _ => PowerProfile::Performance,
        }
    }
}

/// Asusd platform profile (values match the D-Bus property)
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PlatformProfile {
    Balanced = 0,
    Performance = 1,
    Quiet = 2,
}

impl PlatformProfile {
    pub fn name(&self) -> &'static str {
        match self {
            Self::Balanced => "BALANCED",
            Self::Performance => "PERFORMANCE",
            Self::Quiet => "QUIET",
        }
    }

    /// Value accepted by `asusctl --mod-profile`
    pub fn format_value(&self) -> &'static str {
        match self {
            Self::Balanced => "Balanced",
            Self::Performance => "Performance",
            Self::Quiet => "Quiet",
        }
    }

    pub fn from_str(s: &str) -> Option<Self> {
        match s.to_lowercase().as_str() {
            "balanced" => Some(Self::Balanced),
            "performance" => Some(Self::Performance),
            "quiet" => Some(Self::Quiet),
            _ => None,
        }
    }
}

/// Value written to the ACPI platform profile file
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PowerProfile {
    Balanced,
    Performance,
    Quiet,
}

impl PowerProfile {
    pub fn as_str(&self) -> &'static str {
        match self {
            Self::Balanced => "balanced",
            Self::Performance => "performance",
            Self::Quiet => "quiet",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CpuGovernor {
    Powersave,
    Performance,
}

impl CpuGovernor {
    pub fn as_str(&self) -> &'static str {
        match self {
            Self::Powersave => "powersave",
            Self::Performance => "performance",
        }
    }
}

#[cfg(test)]
mod tests {
    use super::PerformanceProfile::*;

    #[test]
    fn profile_cycle_matches_cpp() {
        assert_eq!(Quiet.next(true, true), Balanced);
        assert_eq!(Balanced.next(true, true), Performance);
        assert_eq!(Performance.next(true, true), Smart);
        assert_eq!(Smart.next(true, true), Quiet);
        assert_eq!(Performance.next(false, false), Performance);
        assert_eq!(Quiet.previous(false), Quiet);
    }
}
