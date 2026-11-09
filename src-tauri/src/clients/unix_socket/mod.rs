//! Unix socket clients. Holds the shared client logic (framing, reconnection, keep-alive).

pub mod rog_perf_tuner_client;
pub mod steam_client;

use std::{
    collections::HashMap,
    io::Write,
    net::Shutdown,
    os::unix::net::UnixStream,
    sync::{
        Arc, Mutex,
        atomic::{AtomicBool, Ordering},
        mpsc::{Sender, channel},
    },
    thread,
    time::Duration,
};

use log::{debug, error};
use noyalib::Value;

use crate::{
    models::socket::{UnixMessage, read_frame, write_frame},
    utils::sleep_ms,
};

/// Sends a single request through a fresh connection and waits for its response.
pub fn invoke_once(path: &str, method: &str, data: Vec<Value>, timeout_ms: u64) -> Result<Vec<Value>, String> {
    let mut stream = UnixStream::connect(path).map_err(|e| format!("Couldn't connect to {path}: {e}"))?;
    stream
        .set_read_timeout(Some(Duration::from_millis(timeout_ms)))
        .map_err(|e| e.to_string())?;
    let req = UnixMessage::request(method, data);
    write_frame(&mut stream, &req.to_yaml()).map_err(|e| e.to_string())?;
    loop {
        let frame = read_frame(&mut stream)
            .map_err(|_| "UnixSocketTimeoutError".to_string())?
            .ok_or("Connection closed")?;
        let msg = UnixMessage::from_yaml(&frame)?;
        if msg.kind == "RESPONSE" && msg.id == req.id {
            if let Some(err) = msg.error.filter(|e| !e.is_empty()) {
                return Err(err);
            }
            return Ok(msg.data);
        }
    }
}

type EventHandler = Box<dyn Fn(&UnixMessage) + Send + Sync>;

/// Persistent connection with automatic reconnection and keep-alive pings.
pub struct PersistentSocketClient {
    path: String,
    name: String,
    stream: Mutex<Option<UnixStream>>,
    pending: Mutex<HashMap<String, Sender<UnixMessage>>>,
    connected: AtomicBool,
    on_event: EventHandler,
    on_connect: Box<dyn Fn() + Send + Sync>,
    on_disconnect: Box<dyn Fn() + Send + Sync>,
}

impl PersistentSocketClient {
    pub fn start(
        path: &str,
        name: &str,
        on_event: EventHandler,
        on_connect: Box<dyn Fn() + Send + Sync>,
        on_disconnect: Box<dyn Fn() + Send + Sync>,
    ) -> Arc<Self> {
        let client = Arc::new(Self {
            path: path.to_string(),
            name: name.to_string(),
            stream: Mutex::new(None),
            pending: Mutex::new(HashMap::new()),
            connected: AtomicBool::new(false),
            on_event,
            on_connect,
            on_disconnect,
        });
        let c = client.clone();
        thread::spawn(move || c.connection_loop());
        client
    }

    pub fn connected(&self) -> bool {
        self.connected.load(Ordering::SeqCst)
    }

    fn connection_loop(self: Arc<Self>) {
        loop {
            let stream = match UnixStream::connect(&self.path) {
                Ok(s) => s,
                Err(e) => {
                    debug!(target: "UnixSocketClient", "[{}] Connect error: {e}", self.name);
                    sleep_ms(3000);
                    continue;
                }
            };
            let reader = match stream.try_clone() {
                Ok(r) => r,
                Err(_) => continue,
            };
            *self.stream.lock().unwrap() = Some(stream);
            self.connected.store(true, Ordering::SeqCst);

            let me = self.clone();
            let read_thread = thread::spawn(move || me.read_loop(reader));
            (self.on_connect)();

            // Keep-alive
            while self.connected() {
                for _ in 0..50 {
                    if !self.connected() {
                        break;
                    }
                    sleep_ms(100);
                }
                if self.connected() && self.invoke("ping", vec![], 1000).is_err() {
                    debug!(target: "UnixSocketClient", "[{}] Lost connection", self.name);
                    break;
                }
            }

            self.disconnect();
            let _ = read_thread.join();
            (self.on_disconnect)();
            sleep_ms(1000);
        }
    }

    fn disconnect(&self) {
        self.connected.store(false, Ordering::SeqCst);
        if let Some(s) = self.stream.lock().unwrap().take() {
            let _ = s.shutdown(Shutdown::Both);
        }
        self.pending.lock().unwrap().clear();
    }

    fn read_loop(&self, mut reader: UnixStream) {
        loop {
            match read_frame(&mut reader) {
                Ok(Some(frame)) => match UnixMessage::from_yaml(&frame) {
                    Ok(msg) if msg.kind == "RESPONSE" => {
                        if let Some(tx) = self.pending.lock().unwrap().remove(&msg.id) {
                            let _ = tx.send(msg);
                        }
                    }
                    Ok(msg) if msg.kind == "EVENT" => (self.on_event)(&msg),
                    Ok(_) => {}
                    Err(e) => error!(target: "UnixSocketClient", "[{}] Invalid message: {e}", self.name),
                },
                _ => {
                    debug!(target: "UnixSocketClient", "[{}] Server closed the connection", self.name);
                    self.connected.store(false, Ordering::SeqCst);
                    break;
                }
            }
        }
    }

    pub fn invoke(&self, method: &str, data: Vec<Value>, timeout_ms: u64) -> Result<Vec<Value>, String> {
        let req = UnixMessage::request(method, data);
        let (tx, rx) = channel();
        self.pending.lock().unwrap().insert(req.id.clone(), tx);
        {
            let mut guard = self.stream.lock().unwrap();
            let stream = guard.as_mut().ok_or("Not connected")?;
            if write_frame(stream, &req.to_yaml()).is_err() {
                error!(target: "UnixSocketClient", "[{}] Error sending message", self.name);
            }
            let _ = stream.flush();
        }
        match rx.recv_timeout(Duration::from_millis(timeout_ms)) {
            Ok(resp) => match resp.error.filter(|e| !e.is_empty()) {
                Some(err) => Err(err),
                None => Ok(resp.data),
            },
            Err(_) => {
                self.pending.lock().unwrap().remove(&req.id);
                if method != "ping" {
                    error!(target: "UnixSocketClient", "[{}] No response for {method} after {timeout_ms} ms", self.name);
                }
                Err("UnixSocketTimeoutError".into())
            }
        }
    }
}
