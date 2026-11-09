//! asusctl: fan curves and aura.

use std::{collections::HashMap, sync::LazyLock};

use crate::{
    clients::{ClientResult, shell::CmdClient},
    models::{hardware::FanCurveData, performance::PlatformProfile},
};

pub struct AsusCtlClient {
    cmd: CmdClient,
}

pub static ASUSCTL: LazyLock<AsusCtlClient> = LazyLock::new(|| AsusCtlClient {
    cmd: CmdClient::new("asusctl", "AsusCtlClient"),
});

impl AsusCtlClient {
    pub fn available(&self) -> bool {
        self.cmd.available()
    }

    pub fn turn_off_aura(&self) {
        let _ = self.cmd.run("aura effect static --colour 000000", true, false);
    }

    fn curves_output(&self, profile: PlatformProfile) -> ClientResult<String> {
        let out = self
            .cmd
            .run(&format!("fan-curve --mod-profile {}", profile.format_value()), false, false)?
            .stdout;
        Ok(out.find('[').map(|pos| out[pos..].to_string()).unwrap_or_default())
    }

    pub fn fan_curve_data(&self, profile: PlatformProfile) -> ClientResult<HashMap<String, FanCurveData>> {
        Ok(FanCurveData::parse_curves(&self.curves_output(profile)?))
    }

    pub fn fans(&self, profile: PlatformProfile) -> ClientResult<Vec<String>> {
        Ok(FanCurveData::parse_fans(&self.curves_output(profile)?))
    }

    pub fn set_curves_to_defaults(&self, profile: PlatformProfile) -> ClientResult<()> {
        self.cmd
            .run(&format!("fan-curve --mod-profile {} --default", profile.format_value()), false, false)
            .map(|_| ())
    }

    pub fn set_fan_curves_enabled(&self, profile: PlatformProfile, enabled: bool) -> ClientResult<()> {
        self.cmd
            .run(
                &format!("fan-curve --mod-profile {} --enable-fan-curves {enabled}", profile.format_value()),
                false,
                false,
            )
            .map(|_| ())
    }

    pub fn set_fan_curve_string_data(&self, profile: PlatformProfile, fan: &str, data: &str) -> ClientResult<()> {
        self.cmd
            .run(
                &format!("fan-curve --mod-profile {} --fan {fan} --data {data}", profile.format_value()),
                false,
                false,
            )
            .map(|_| ())
    }
}
