//! asus-armoury firmware attributes. Holds the shared attribute client; each attribute has its own file.

pub mod boot_sound_client;
pub mod nv_boost_client;
pub mod nv_temp_client;
pub mod panel_overdrive_client;
pub mod pl1_spl_client;
pub mod pl2_sppt_client;
pub mod pl3_fppt_client;

use std::{fs, path::Path};

use log::error;

use crate::clients::{ClientError, ClientResult, file::FileClient};

/// asus-armoury firmware attribute (`current_value`, `min_value`, `max_value`, `possible_values`)
#[derive(Debug, Clone)]
pub struct ArmouryClient {
    attribute_path: String,
    file: FileClient,
}

impl ArmouryClient {
    pub fn new(attribute_path: &str) -> Self {
        Self {
            attribute_path: attribute_path.to_string(),
            file: FileClient::new(&format!("{attribute_path}/current_value"), true),
        }
    }

    pub fn available(&self) -> bool {
        self.file.available()
    }

    pub fn current_value(&self) -> ClientResult<i32> {
        let v = self.file.read()?;
        v.trim().parse().map_err(|_| {
            error!(target: "ArmouryClient", "stoi failed for value '{}'", v.trim());
            ClientError::Parse(v.trim().to_string())
        })
    }

    pub fn set_current_value(&self, value: i32) -> ClientResult<()> {
        let (min, max) = (self.min_value()?, self.max_value()?);
        if value < min || value > max {
            return Err(ClientError::OutOfRange { value, min, max });
        }
        self.file.write(&value.to_string())
    }

    fn bound(&self, file: &str, possible_last: bool) -> ClientResult<i32> {
        let direct = format!("{}/{file}", self.attribute_path);
        let content = if Path::new(&direct).exists() {
            fs::read_to_string(&direct).unwrap_or_default()
        } else {
            let possible = format!("{}/possible_values", self.attribute_path);
            if !Path::new(&possible).exists() {
                return Ok(0);
            }
            let all = fs::read_to_string(possible).unwrap_or_default();
            let parts: Vec<&str> = all.split(';').collect();
            if possible_last { parts.last().copied() } else { parts.first().copied() }
                .unwrap_or("0")
                .to_string()
        };
        content.trim().parse().map_err(|_| ClientError::Parse(content.trim().to_string()))
    }

    pub fn max_value(&self) -> ClientResult<i32> {
        self.bound("max_value", true)
    }

    pub fn min_value(&self) -> ClientResult<i32> {
        self.bound("min_value", false)
    }
}

/// Builds an armoury client for an optional (detected) attribute path.
pub fn armoury(path: &Option<String>) -> Option<ArmouryClient> {
    path.as_deref().map(ArmouryClient::new).filter(|c| c.available())
}
