//! RCCDeckyCompanion plugin: game launch/stop events and app details.

use std::sync::{Arc, LazyLock};

use crate::{
    clients::unix_socket::PersistentSocketClient,
    models::{
        socket::{int_value, str_value, value_to_i64, value_to_string},
        steam::{SteamGameDetails, SteamGameDetailsMap},
    },
    utils::{
        constants,
        event_bus::{self, Event},
    },
};

pub static STEAM: LazyLock<Arc<PersistentSocketClient>> = LazyLock::new(|| {
    PersistentSocketClient::start(
        &constants::RCCDC_SOCKET_PATH,
        "SteamClient",
        Box::new(|msg| {
            let id = msg.data.first().and_then(value_to_i64).unwrap_or(0) as u32;
            let name = msg.data.get(1).map(value_to_string).unwrap_or_default();
            match msg.name.as_str() {
                "launch_game" => {
                    let pid = msg.data.get(2).and_then(value_to_i64).unwrap_or(0) as i32;
                    event_bus::emit(Event::SteamGameLaunch { id, name, pid });
                }
                "stop_game" => event_bus::emit(Event::SteamGameStop { id, name }),
                _ => {}
            }
        }),
        Box::new(|| event_bus::emit(Event::SteamConnected)),
        Box::new(|| event_bus::emit(Event::SteamDisconnected)),
    )
});

pub fn apps_details(app_ids: &[u32]) -> Result<Vec<SteamGameDetails>, String> {
    let data = app_ids.iter().map(|id| int_value(*id as i64)).collect();
    let res = STEAM.invoke("get_apps_details", data, 3000)?;
    let yaml = res.first().map(value_to_string).ok_or("Empty response")?;
    let map: SteamGameDetailsMap = noyalib::from_str(&yaml).map_err(|e| e.to_string())?;
    Ok(map.into_values().collect())
}

pub fn set_launch_options(app_id: u32, opts: &str) -> Result<(), String> {
    STEAM
        .invoke("set_launch_options", vec![int_value(app_id as i64), str_value(opts)], 3000)
        .map(|_| ())
}
