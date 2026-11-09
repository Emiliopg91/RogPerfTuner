use std::{collections::HashMap, sync::LazyLock};

use regex::Regex;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
#[serde(try_from = "u32", into = "u32")]
pub enum BatteryThreshold {
    #[default]
    Ct100 = 100,
    Ct75 = 75,
    Ct50 = 50,
}

impl BatteryThreshold {
    pub const ALL: [BatteryThreshold; 3] = [Self::Ct100, Self::Ct75, Self::Ct50];

    pub fn value(&self) -> u32 {
        *self as u32
    }
}

impl From<BatteryThreshold> for u32 {
    fn from(v: BatteryThreshold) -> Self {
        v.value()
    }
}

impl TryFrom<u32> for BatteryThreshold {
    type Error = String;
    fn try_from(v: u32) -> Result<Self, Self::Error> {
        match v {
            100 => Ok(Self::Ct100),
            75 => Ok(Self::Ct75),
            50 => Ok(Self::Ct50),
            _ => Err(format!("Invalid value '{v}'")),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
#[serde(try_from = "u32", into = "u32")]
pub enum RgbBrightness {
    #[default]
    Max = 100,
    High = 67,
    Medium = 50,
    Low = 33,
    Off = 0,
}

impl RgbBrightness {
    /// Highest to lowest, as shown in the GUI
    pub const ALL: [RgbBrightness; 5] = [Self::Max, Self::High, Self::Medium, Self::Low, Self::Off];

    pub fn value(&self) -> u32 {
        *self as u32
    }

    pub fn name(&self) -> &'static str {
        match self {
            Self::Max => "MAX",
            Self::High => "HIGH",
            Self::Medium => "MEDIUM",
            Self::Low => "LOW",
            Self::Off => "OFF",
        }
    }

    pub fn next(&self) -> Self {
        match self {
            Self::Medium => Self::High,
            Self::Low => Self::Medium,
            Self::Off => Self::Low,
            _ => Self::Max,
        }
    }

    pub fn previous(&self) -> Self {
        match self {
            Self::Max => Self::High,
            Self::High => Self::Medium,
            Self::Medium => Self::Low,
            _ => Self::Off,
        }
    }
}

impl From<RgbBrightness> for u32 {
    fn from(v: RgbBrightness) -> Self {
        v.value()
    }
}

impl TryFrom<u32> for RgbBrightness {
    type Error = String;
    fn try_from(v: u32) -> Result<Self, Self::Error> {
        match v {
            100 => Ok(Self::Max),
            67 => Ok(Self::High),
            50 => Ok(Self::Medium),
            33 => Ok(Self::Low),
            0 => Ok(Self::Off),
            _ => Err(format!("Invalid value '{v}'")),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct UsbIdentifier {
    pub id_vendor: String,
    pub id_product: String,
    pub name: String,
}

impl PartialEq for UsbIdentifier {
    fn eq(&self, other: &Self) -> bool {
        self.id_vendor == other.id_vendor && self.id_product == other.id_product
    }
}

static FAN_BLOCK_RE: LazyLock<Regex> = LazyLock::new(|| {
    Regex::new(r"\(\s*fan:\s*([A-Za-z0-9_]+)\s*,\s*pwm:\s*\(([^)]*)\)\s*,\s*temp:\s*\(([^)]*)\)").unwrap()
});

/// Fan curve as temperature (ºC) / speed (%) points
#[derive(Debug, Clone, Default, PartialEq, Serialize, Deserialize)]
pub struct FanCurveData {
    pub perc: Vec<i32>,
    pub temp: Vec<i32>,
}

impl FanCurveData {
    fn parse_numbers(s: &str) -> Vec<i32> {
        s.split(',').filter_map(|t| t.trim().parse().ok()).collect()
    }

    /// Parses the output of `asusctl fan-curve --mod-profile <profile>`
    pub fn parse_curves(input: &str) -> HashMap<String, FanCurveData> {
        FAN_BLOCK_RE
            .captures_iter(input)
            .map(|cap| {
                let perc = Self::parse_numbers(&cap[2]).into_iter().map(|p| (100 * p) / 255).collect();
                (cap[1].to_string(), FanCurveData { perc, temp: Self::parse_numbers(&cap[3]) })
            })
            .collect()
    }

    /// Names of the fans in an `asusctl fan-curve` output, in order
    pub fn parse_fans(input: &str) -> Vec<String> {
        FAN_BLOCK_RE.captures_iter(input).map(|cap| cap[1].to_string()).collect()
    }

    /// Parses `30c:10%,40c:20%,...`
    pub fn from_data(data: &str) -> Self {
        let mut res = FanCurveData::default();
        for entry in data.split(',').filter(|e| !e.trim().is_empty()) {
            if let Some((t, p)) = entry.split_once(':') {
                let t = t.trim().trim_end_matches(['c', 'C']).parse().unwrap_or(0);
                let p = p.trim().trim_end_matches('%').parse().unwrap_or(0);
                res.temp.push(t);
                res.perc.push(p);
            }
        }
        res
    }

    pub fn to_data(&self) -> String {
        self.temp
            .iter()
            .zip(self.perc.iter())
            .map(|(t, p)| format!("{t}c:{p}%"))
            .collect::<Vec<_>>()
            .join(",")
    }

    /// Same curve, allowing 1% of difference: asusd stores PWM (0-255) and reports it back rounded to %.
    pub fn approx_eq(&self, other: &FanCurveData) -> bool {
        self.temp == other.temp
            && self.perc.len() == other.perc.len()
            && self.perc.iter().zip(&other.perc).all(|(a, b)| (a - b).abs() <= 1)
    }

    /// Spreads out consecutive points with the same temperature (asusctl rejects them)
    pub fn normalize(&mut self) {
        for i in 1..self.temp.len() {
            if self.temp[i - 1] == self.temp[i] {
                let prev_tmp = self.temp[i - 1];
                let next_tmp = if i + 1 < self.temp.len() { self.temp[i + 1] } else { self.temp[i] + 4 };
                self.temp[i] = (prev_tmp + next_tmp) / 2;

                let prev_per = self.perc[i - 1];
                let next_per = if i + 1 < self.perc.len() { self.perc[i + 1] } else { (self.perc[i] + 5).min(100) };
                self.perc[i] = (prev_per + next_per) / 2;
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fan_curve_roundtrip_and_normalize() {
        let data = "63c:0%,63c:0%,67c:8%,71c:10%,75c:14%,75c:14%,79c:24%,83c:30%";
        let mut curve = FanCurveData::from_data(data);
        assert_eq!(curve.to_data(), data);
        curve.normalize();
        assert_eq!(curve.to_data(), "63c:0%,65c:4%,67c:8%,71c:10%,75c:14%,77c:19%,79c:24%,83c:30%");
    }

    #[test]
    fn approx_eq_tolerates_pwm_rounding() {
        let wanted = FanCurveData::from_data("50c:0%,62c:8%,66c:14%");
        // 8% -> pwm 20 -> reported 7%, 14% -> pwm 36 -> reported 14%
        let live = FanCurveData { temp: vec![50, 62, 66], perc: vec![0, 7, 14] };
        assert!(live.approx_eq(&wanted));
        let other = FanCurveData { temp: vec![50, 62, 66], perc: vec![0, 20, 14] };
        assert!(!other.approx_eq(&wanted));
    }

    #[test]
    fn parses_asusctl_output() {
        let out = "[(fan: CPU, pwm: (0, 20, 35), temp: (50, 62, 66), enabled: true), (fan: GPU, pwm: (255, 255, 255), temp: (1, 2, 3), enabled: false)]";
        let curves = FanCurveData::parse_curves(out);
        assert_eq!(curves["CPU"].perc, vec![0, 7, 13]);
        assert_eq!(curves["GPU"].temp, vec![1, 2, 3]);
        assert_eq!(FanCurveData::parse_fans(out), vec!["CPU", "GPU"]);
    }
}
