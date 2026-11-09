//! `config.yaml` model. Keeps the format of previous releases so existing configurations are preserved.

use std::collections::BTreeMap;

use serde::{Deserialize, Deserializer, Serialize, Serializer, ser::SerializeMap};

use crate::models::{
    hardware::{BatteryThreshold, RgbBrightness},
    lenient,
    performance::PerformanceProfile,
};

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase", default)]
pub struct Application {
    pub asked_install_rccdc: bool,
    pub start_minimized: bool,
}

impl Default for Application {
    fn default() -> Self {
        Self {
            asked_install_rccdc: false,
            start_minimized: true,
        }
    }
}

#[derive(Debug, Clone, Default, Serialize, Deserialize, PartialEq)]
#[serde(default)]
pub struct EffectConfig {
    pub color: String,
}

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
#[serde(default)]
pub struct Aura {
    pub brightness: RgbBrightness,
    #[serde(skip_serializing_if = "BTreeMap::is_empty")]
    pub config: BTreeMap<String, EffectConfig>,
    #[serde(rename = "effect", skip_serializing_if = "Option::is_none")]
    pub last_effect: Option<String>,
}

#[derive(Debug, Clone, Default, Serialize, Deserialize, PartialEq)]
pub struct FanCurve {
    #[serde(default)]
    pub current: String,
    #[serde(default, alias = "factory")]
    pub presets: String,
}

fn is_none_scheduler(s: &str) -> bool {
    s == "none"
}

fn default_none() -> String {
    "none".to_string()
}

fn deserialize_profile<'de, D: Deserializer<'de>>(d: D) -> Result<PerformanceProfile, D::Error> {
    let s = String::deserialize(d)?;
    Ok(PerformanceProfile::from_str(&s).unwrap_or_default())
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Performance {
    #[serde(default, deserialize_with = "deserialize_profile")]
    pub profile: PerformanceProfile,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub scheduler: Option<String>,
    #[serde(default = "default_none", skip_serializing_if = "is_none_scheduler")]
    pub ssd_scheduler: String,
}

impl Default for Performance {
    fn default() -> Self {
        Self {
            profile: PerformanceProfile::Smart,
            scheduler: None,
            ssd_scheduler: default_none(),
        }
    }
}

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
#[serde(rename_all = "camelCase", default)]
pub struct Platform {
    pub charge_limit: BatteryThreshold,
    /// profile -> fan -> curve
    #[serde(skip_serializing_if = "BTreeMap::is_empty")]
    pub curves: BTreeMap<String, BTreeMap<String, FanCurve>>,
    pub performance: Performance,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
#[serde(rename_all = "snake_case")]
pub enum MangoHudLevel {
    #[default]
    NoDisplay = 0,
    FpsOnly = 1,
    Horizontal = 2,
    Extended = 3,
    Detailed = 4,
}

impl MangoHudLevel {
    pub const ALL: [MangoHudLevel; 5] = [Self::NoDisplay, Self::FpsOnly, Self::Horizontal, Self::Extended, Self::Detailed];
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
#[serde(rename_all = "snake_case")]
pub enum ComputerType {
    #[default]
    Computer,
    SteamDeck,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Default)]
#[serde(rename_all = "lowercase")]
pub enum WineSyncOption {
    #[default]
    Auto,
    Ntsync,
    Fsync,
    Esync,
    None,
}

/// Per game configuration
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct GameEntry {
    pub name: String,
    pub args: Option<String>,
    pub env: Option<String>,
    pub gpu: Option<String>,
    pub scheduler: Option<String>,
    pub metrics_level: MangoHudLevel,
    #[serde(rename = "overlayId")]
    pub overlay_id: Option<String>,
    pub proton: bool,
    pub device: ComputerType,
    pub sync: WineSyncOption,
    pub wrappers: Option<String>,
}

impl Default for GameEntry {
    fn default() -> Self {
        Self {
            name: String::new(),
            args: None,
            env: None,
            gpu: None,
            scheduler: None,
            metrics_level: MangoHudLevel::NoDisplay,
            overlay_id: None,
            proton: true,
            device: ComputerType::Computer,
            sync: WineSyncOption::Auto,
            wrappers: None,
        }
    }
}

/// Raw YAML layout of a game entry, where optional and default values are omitted.
#[derive(Serialize, Deserialize, Default)]
#[serde(default)]
struct GameEntryYaml {
    #[serde(skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    name: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    args: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    env: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    gpu: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    metrics: Option<String>,
    #[serde(rename = "overlayId", skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    overlay_id: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    proton: Option<bool>,
    #[serde(skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    device: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    sync: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    scheduler: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none", deserialize_with = "lenient::opt_string")]
    wrappers: Option<String>,
}

fn non_empty(s: &Option<String>) -> Option<String> {
    s.clone().filter(|v| !v.is_empty())
}

fn enum_from_str<T: for<'de> Deserialize<'de>>(s: &str) -> Option<T> {
    serde_json::from_value(serde_json::Value::String(s.to_lowercase())).ok()
}

fn enum_to_str<T: Serialize>(v: &T) -> String {
    serde_json::to_value(v)
        .ok()
        .and_then(|v| v.as_str().map(|s| s.to_string()))
        .unwrap_or_default()
}

impl GameEntry {
    pub fn to_yaml_value(&self) -> impl Serialize {
        GameEntryYaml {
            name: Some(self.name.clone()),
            args: non_empty(&self.args),
            env: non_empty(&self.env),
            gpu: non_empty(&self.gpu),
            metrics: (self.metrics_level != MangoHudLevel::NoDisplay).then(|| enum_to_str(&self.metrics_level)),
            overlay_id: non_empty(&self.overlay_id),
            proton: (!self.proton).then_some(false),
            device: (self.proton && self.device != ComputerType::Computer).then(|| enum_to_str(&self.device)),
            sync: (self.proton && self.sync != WineSyncOption::Auto).then(|| enum_to_str(&self.sync)),
            scheduler: non_empty(&self.scheduler),
            wrappers: non_empty(&self.wrappers),
        }
    }

    fn from_yaml(raw: GameEntryYaml) -> Self {
        let mut entry = GameEntry {
            name: raw.name.unwrap_or_default(),
            args: raw.args,
            env: raw.env,
            gpu: raw.gpu,
            scheduler: raw.scheduler,
            overlay_id: raw.overlay_id,
            wrappers: raw.wrappers,
            ..Default::default()
        };
        if let Some(p) = raw.proton {
            entry.proton = p;
        }
        if let Some(d) = raw.device.as_deref().and_then(enum_from_str) {
            entry.device = d;
        }
        if let Some(s) = raw.sync.as_deref().and_then(enum_from_str) {
            entry.sync = s;
        }
        if let Some(m) = raw.metrics.as_deref().and_then(enum_from_str) {
            entry.metrics_level = m;
        }
        entry
    }

    /// Parses a game entry from a generic YAML value (used for the suggestions file).
    pub fn from_value(value: &noyalib::Value) -> Option<Self> {
        let yaml = noyalib::to_string(value).ok()?;
        noyalib::from_str::<GameEntryYaml>(&yaml).ok().map(Self::from_yaml)
    }
}

/// Map key accepting both integer and string YAML scalars (app ids).
#[derive(PartialEq, Eq, PartialOrd, Ord)]
struct AppIdKey(u32);

impl<'de> Deserialize<'de> for AppIdKey {
    fn deserialize<D: Deserializer<'de>>(d: D) -> Result<Self, D::Error> {
        struct V;
        impl serde::de::Visitor<'_> for V {
            type Value = AppIdKey;
            fn expecting(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
                f.write_str("an app id")
            }
            fn visit_u64<E: serde::de::Error>(self, v: u64) -> Result<AppIdKey, E> {
                u32::try_from(v).map(AppIdKey).map_err(E::custom)
            }
            fn visit_i64<E: serde::de::Error>(self, v: i64) -> Result<AppIdKey, E> {
                u32::try_from(v).map(AppIdKey).map_err(E::custom)
            }
            fn visit_str<E: serde::de::Error>(self, v: &str) -> Result<AppIdKey, E> {
                v.trim().parse().map(AppIdKey).map_err(E::custom)
            }
        }
        d.deserialize_any(V)
    }
}

struct GamesMap;

impl GamesMap {
    fn serialize<S: Serializer>(games: &BTreeMap<u32, GameEntry>, s: S) -> Result<S::Ok, S::Error> {
        let mut map = s.serialize_map(Some(games.len()))?;
        for (k, v) in games {
            // Integer keys, like yaml-cpp writes them
            map.serialize_entry(&noyalib::to_value(k).map_err(serde::ser::Error::custom)?, &v.to_yaml_value())?;
        }
        map.end()
    }

    fn deserialize<'de, D: Deserializer<'de>>(d: D) -> Result<BTreeMap<u32, GameEntry>, D::Error> {
        let raw = Option::<BTreeMap<AppIdKey, GameEntryYaml>>::deserialize(d)?.unwrap_or_default();
        Ok(raw.into_iter().map(|(k, v)| (k.0, GameEntry::from_yaml(v))).collect())
    }
}

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
#[serde(default)]
pub struct RootConfig {
    pub application: Application,
    pub aura: Aura,
    #[serde(serialize_with = "GamesMap::serialize", deserialize_with = "GamesMap::deserialize")]
    pub games: BTreeMap<u32, GameEntry>,
    pub logger: BTreeMap<String, String>,
    pub platform: Platform,
}

#[cfg(test)]
mod tests {
    use super::*;

    const SAMPLE: &str = r##"application:
  askedInstallRccdc: true
  startMinimized: true
aura:
  brightness: 100
  config:
    Digital rain:
      color: "#00ff00"
  effect: Digital rain
games:
  1030300:
    name: "Hollow Knight: Silksong"
    args: --force-vulkan
    gpu: nvidia
    metrics: extended
    proton: false
    scheduler: Lavd
  1091500:
    name: Cyberpunk 2077
    env: PROTON_VKD3D_HEAP=1
    sync: ntsync
  2048:
    name: 2048
    overlayId: 12345678901234
logger:
  {}
platform:
  chargeLimit: 75
  curves:
    quiet:
      CPU:
        current: 63c:0%,65c:4%
        factory: 63c:0%,63c:0%
  performance:
    profile: low_power
    scheduler: BORE
    ssdScheduler: adios
"##;

    #[test]
    fn parses_cpp_config() {
        let cfg: RootConfig = noyalib::from_str(SAMPLE).unwrap();
        assert!(cfg.application.asked_install_rccdc);
        assert_eq!(cfg.aura.last_effect.as_deref(), Some("Digital rain"));
        assert_eq!(cfg.aura.config["Digital rain"].color, "#00ff00");
        let hk = &cfg.games[&1030300];
        assert_eq!(hk.name, "Hollow Knight: Silksong");
        assert!(!hk.proton);
        assert_eq!(hk.metrics_level, MangoHudLevel::Extended);
        assert_eq!(cfg.games[&1091500].sync, WineSyncOption::Ntsync);
        assert_eq!(cfg.platform.charge_limit, BatteryThreshold::Ct75);
        assert_eq!(cfg.platform.curves["quiet"]["CPU"].presets, "63c:0%,63c:0%");
        assert_eq!(cfg.platform.performance.profile, PerformanceProfile::Quiet);
        assert_eq!(cfg.platform.performance.ssd_scheduler, "adios");
    }

    #[test]
    fn roundtrip_keeps_values() {
        let cfg: RootConfig = noyalib::from_str(SAMPLE).unwrap();
        let out = noyalib::to_string(&cfg).unwrap();
        let again: RootConfig = noyalib::from_str(&out).unwrap();
        assert_eq!(again.games, cfg.games);
        assert_eq!(again.platform.curves, cfg.platform.curves);
        assert!(out.contains("profile: quiet"), "{out}");
        assert!(!out.contains("proton: true"), "{out}");
        // noyalib quotes integer keys; yaml-cpp still reads them as numbers
        assert!(out.contains("1030300"), "{out}");
    }

    /// Parses the real user configuration without modifying it: `cargo test -- --ignored`
    #[test]
    #[ignore]
    fn parses_user_config() {
        let path = format!("{}/.RogPerfTuner/config/config.yaml", std::env::var("HOME").unwrap());
        let content = std::fs::read_to_string(path).unwrap();
        let cfg: RootConfig = noyalib::from_str(&content).unwrap();
        let again: RootConfig = noyalib::from_str(&noyalib::to_string(&cfg).unwrap()).unwrap();
        assert_eq!(again.games, cfg.games);
        assert_eq!(again.platform.curves, cfg.platform.curves);
        println!("{} games, profile {:?}", cfg.games.len(), cfg.platform.performance.profile);
    }

    #[test]
    fn empty_config_gets_defaults() {
        let cfg: RootConfig = noyalib::from_str("{}").unwrap();
        assert!(cfg.application.start_minimized);
        assert_eq!(cfg.platform.performance.ssd_scheduler, "none");
        assert_eq!(cfg.aura.brightness, RgbBrightness::Max);
    }
}
