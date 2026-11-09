//! asusd platform properties on the system bus.

use std::sync::LazyLock;

use log::error;
use zbus::{
    blocking::{Connection, Proxy},
    zvariant::Value,
};

use crate::models::performance::PlatformProfile;

const LOG: &str = "PlatformClient";

/// `xyz.ljones.Platform` interface of asusd
pub struct PlatformClient {
    proxy: Option<Proxy<'static>>,
}

pub static PLATFORM: LazyLock<PlatformClient> = LazyLock::new(|| {
    let proxy = Connection::system()
        .and_then(|conn| Proxy::new_owned(conn, "xyz.ljones.Asusd", "/xyz/ljones", "xyz.ljones.Platform"))
        .map_err(|e| error!(target: LOG, "Couldn't connect to asusd: {e}"))
        .ok();
    PlatformClient { proxy }
});

impl PlatformClient {
    fn set<'a, T: Into<Value<'a>> + 'a>(&self, name: &str, value: T) -> Result<(), String> {
        match &self.proxy {
            Some(p) => p.set_property(name, value).map_err(|e| e.to_string()),
            None => Err("D-Bus service not available: xyz.ljones.Asusd".into()),
        }
    }

    pub fn set_platform_profile(&self, profile: PlatformProfile) -> Result<(), String> {
        self.set("PlatformProfile", profile as u32)
    }

    pub fn set_enable_ppt_group(&self, enable: bool) -> Result<(), String> {
        self.set("EnablePptGroup", enable)
    }

    pub fn set_platform_profile_linked_epp(&self, enable: bool) -> Result<(), String> {
        self.set("PlatformProfileLinkedEpp", enable)
    }

    pub fn set_change_platform_profile_on_battery(&self, enable: bool) -> Result<(), String> {
        self.set("ChangePlatformProfileOnBattery", enable)
    }

    pub fn set_change_platform_profile_on_ac(&self, enable: bool) -> Result<(), String> {
        self.set("ChangePlatformProfileOnAc", enable)
    }
}
