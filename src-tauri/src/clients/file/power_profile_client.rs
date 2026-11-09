//! ACPI platform profile.

use crate::{
    clients::{ClientResult, file::FileClient},
    features::features,
    models::performance::PowerProfile,
};

pub fn set_power_profile(profile: PowerProfile) -> ClientResult<()> {
    match &features().acpi_profile {
        Some(path) => FileClient::new(path, true).write(profile.as_str()),
        None => Ok(()),
    }
}
