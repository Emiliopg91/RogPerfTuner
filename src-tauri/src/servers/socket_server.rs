//! Unix socket server used by the CLI options, keyboard shortcut scripts and the
//! Steam `run` wrapper to talk to the running instance.

use std::{
    fs,
    os::unix::net::{UnixListener, UnixStream},
    thread,
};

use log::{error, info};

use crate::{
    models::socket::{UnixMessage, read_frame, str_value, value_to_string, write_frame},
    services::{openrgb, performance, steam},
    utils::{
        constants,
        event_bus::{self, Event},
        logger::tabbed,
    },
};

const LOG: &str = "SocketServer";

pub fn start() {
    info!(target: LOG, "Initializing socket server");
    tabbed(|| {
        let path = constants::SOCKET_FILE.as_str();
        let _ = fs::remove_file(path);
        match UnixListener::bind(path) {
            Ok(listener) => {
                thread::spawn(move || {
                    for stream in listener.incoming() {
                        match stream {
                            Ok(s) => {
                                thread::spawn(move || handle_client(s));
                            }
                            Err(e) => error!(target: LOG, "Accept failed: {e}"),
                        }
                    }
                });
                event_bus::subscribe(|e| {
                    if let Event::ApplicationShutdown = e {
                        info!(target: LOG, "Stopping socket server");
                        let _ = fs::remove_file(constants::SOCKET_FILE.as_str());
                        info!(target: LOG, "Socket server stopped");
                    }
                });
                info!(target: LOG, "Socket server started on {path}");
            }
            Err(e) => error!(target: LOG, "Failed to bind socket: {e}"),
        }
    });
}

fn handle_client(mut stream: UnixStream) {
    loop {
        let frame = match read_frame(&mut stream) {
            Ok(Some(f)) => f,
            Ok(None) => break,
            Err(e) => {
                error!(target: LOG, "Error reading message: {e}");
                break;
            }
        };
        match UnixMessage::from_yaml(&frame) {
            Ok(req) if req.kind == "REQUEST" => {
                let res = tabbed(|| handle_request(req));
                if write_frame(&mut stream, &res.to_yaml()).is_err() {
                    break;
                }
            }
            Ok(_) => {}
            Err(e) => error!(target: LOG, "YAML parse error: {e}"),
        }
    }
}

fn handle_request(req: UnixMessage) -> UnixMessage {
    let mut res = UnixMessage {
        kind: "RESPONSE".into(),
        data: vec![],
        ..req.clone()
    };
    match req.name.as_str() {
        constants::NEXT_EFF => res.data.push(str_value(&openrgb::get().next_effect())),
        constants::INC_BRIGHT => res.data.push(str_value(openrgb::get().increase_brightness().name())),
        constants::DEC_BRIGHT => res.data.push(str_value(openrgb::get().decrease_brightness().name())),
        constants::PERF_PROF => res.data.push(str_value(performance::get().next_performance_profile().name())),
        constants::SHOW_GUI => event_bus::emit(Event::ShowGui),
        constants::GAME_CFG => {
            let id = req.data.first().map(value_to_string).unwrap_or_default();
            let cfg = steam::get().configuration(&id);
            res.data.push(str_value(&noyalib::to_string(&cfg).unwrap_or_default()));
        }
        _ => res.error = Some("No such method".into()),
    }
    res
}
