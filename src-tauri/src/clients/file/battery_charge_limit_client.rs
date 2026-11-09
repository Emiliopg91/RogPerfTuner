//! Battery charge threshold.

use crate::{
    clients::{ClientError, ClientResult, file::FileClient},
    features::features,
    models::hardware::BatteryThreshold,
};

pub fn set_charge_limit(value: BatteryThreshold) -> ClientResult<()> {
    match &features().bat_limit {
        Some(path) => FileClient::new(path, true).write(&value.value().to_string()),
        None => Err(ClientError::Missing("charge_control_end_threshold".into())),
    }
}
