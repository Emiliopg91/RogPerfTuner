//! Lenient string deserialization: yaml-cpp and PyYAML write numeric looking strings unquoted
//! (`MANGOHUD_DLSYM: 1`, `name: 1979`), which strict deserializers reject as non-strings.

use std::{collections::HashMap, fmt};

use serde::{
    Deserialize, Deserializer,
    de::{self, Visitor},
};

struct ScalarString;

impl Visitor<'_> for ScalarString {
    type Value = String;

    fn expecting(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.write_str("a scalar")
    }
    fn visit_str<E: de::Error>(self, v: &str) -> Result<String, E> {
        Ok(v.to_string())
    }
    fn visit_string<E: de::Error>(self, v: String) -> Result<String, E> {
        Ok(v)
    }
    fn visit_i64<E: de::Error>(self, v: i64) -> Result<String, E> {
        Ok(v.to_string())
    }
    fn visit_u64<E: de::Error>(self, v: u64) -> Result<String, E> {
        Ok(v.to_string())
    }
    fn visit_f64<E: de::Error>(self, v: f64) -> Result<String, E> {
        Ok(v.to_string())
    }
    fn visit_bool<E: de::Error>(self, v: bool) -> Result<String, E> {
        Ok(v.to_string())
    }
    fn visit_unit<E: de::Error>(self) -> Result<String, E> {
        Ok(String::new())
    }
}

/// String accepting any YAML scalar.
#[derive(Debug, Clone, Default, PartialEq, Eq, Hash)]
pub struct LenientString(pub String);

impl<'de> Deserialize<'de> for LenientString {
    fn deserialize<D: Deserializer<'de>>(d: D) -> Result<Self, D::Error> {
        d.deserialize_any(ScalarString).map(LenientString)
    }
}

pub fn string<'de, D: Deserializer<'de>>(d: D) -> Result<String, D::Error> {
    Ok(Option::<LenientString>::deserialize(d)?.map(|s| s.0).unwrap_or_default())
}

pub fn opt_string<'de, D: Deserializer<'de>>(d: D) -> Result<Option<String>, D::Error> {
    Ok(Option::<LenientString>::deserialize(d)?.map(|s| s.0))
}

pub fn string_map<'de, D: Deserializer<'de>>(d: D) -> Result<HashMap<String, String>, D::Error> {
    Ok(Option::<HashMap<LenientString, LenientString>>::deserialize(d)?
        .unwrap_or_default()
        .into_iter()
        .map(|(k, v)| (k.0, v.0))
        .collect())
}

pub fn string_vec<'de, D: Deserializer<'de>>(d: D) -> Result<Vec<String>, D::Error> {
    Ok(Option::<Vec<LenientString>>::deserialize(d)?
        .unwrap_or_default()
        .into_iter()
        .map(|s| s.0)
        .collect())
}

#[cfg(test)]
mod tests {
    use crate::models::steam::SteamGameConfig;

    #[test]
    fn yaml_cpp_game_config() {
        let yaml = "environment:\n  MANGOHUD_DLSYM: 1\n  SteamDeck: 0\n  PROTON_VKD3D_HEAP: \"1\"\nwrappers:\n  - /usr/bin/systemd-inhibit\n  - --who\nparameters: 1080\n";
        let cfg: SteamGameConfig = noyalib::from_str(yaml).unwrap();
        assert_eq!(cfg.environment["MANGOHUD_DLSYM"], "1");
        assert_eq!(cfg.environment["SteamDeck"], "0");
        assert_eq!(cfg.parameters, "1080");
        assert_eq!(cfg.wrappers.len(), 2);
    }
}
