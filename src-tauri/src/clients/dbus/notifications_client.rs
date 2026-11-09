//! Desktop notifications on the session bus.

use std::{
    collections::HashMap,
    sync::{LazyLock, Mutex},
};

use log::error;
use zbus::{
    blocking::{Connection, Proxy},
    zvariant::Value,
};

use crate::utils::constants;

/// Desktop notifications, replacing the previous toast when shown again (`Toaster`)
pub struct Toaster {
    proxy: Option<Proxy<'static>>,
    last_id: Mutex<u32>,
}

pub static TOASTER: LazyLock<Toaster> = LazyLock::new(|| {
    let proxy = Connection::session()
        .and_then(|conn| {
            Proxy::new_owned(
                conn,
                "org.freedesktop.Notifications",
                "/org/freedesktop/Notifications",
                "org.freedesktop.Notifications",
            )
        })
        .map_err(|e| error!(target: "Toaster", "Couldn't connect to notifications service: {e}"))
        .ok();
    Toaster {
        proxy,
        last_id: Mutex::new(0),
    }
});

impl Toaster {
    pub fn show_toast(&self, message: &str) {
        let Some(proxy) = &self.proxy else {
            return;
        };
        let mut last = self.last_id.lock().unwrap();
        if *last > 0 {
            let _: Result<(), _> = proxy.call("CloseNotification", &(*last,));
        }
        let hints: HashMap<&str, Value> = HashMap::new();
        let res: Result<u32, _> = proxy.call(
            "Notify",
            &(
                " ",
                0u32,
                constants::ASSET_ICON_FILE.as_str(),
                constants::APP_NAME,
                format!("{message}\r\n"),
                Vec::<&str>::new(),
                hints,
                3000i32,
            ),
        );
        match res {
            Ok(id) => *last = id,
            Err(e) => error!(target: "Toaster", "Error showing notification: {e}"),
        }
    }
}
