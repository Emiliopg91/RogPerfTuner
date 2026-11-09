//! Minimal OpenRGB SDK network protocol client (protocol version 3), replacing `submodules/OpenRGB-cppSDK`.
//!
//! Every packet is a 16 byte header (`ORGB`, device index, packet id, body size, little endian) plus a body.

use std::{
    io::{Read, Write},
    net::TcpStream,
    sync::{
        Mutex,
        atomic::{AtomicBool, Ordering},
    },
    time::Duration,
};

const PROTOCOL_VERSION: u32 = 3;

const REQUEST_CONTROLLER_COUNT: u32 = 0;
const REQUEST_CONTROLLER_DATA: u32 = 1;
const REQUEST_PROTOCOL_VERSION: u32 = 40;
const SET_CLIENT_NAME: u32 = 50;
const RGBCONTROLLER_UPDATELEDS: u32 = 1050;
const RGBCONTROLLER_UPDATEMODE: u32 = 1101;

pub const DEVICE_TYPE_KEYBOARD: u32 = 5;
pub const DEVICE_TYPE_MOUSE: u32 = 6;
pub const DEVICE_TYPE_LAPTOP: u32 = 19;
pub const ZONE_TYPE_MATRIX: u32 = 2;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct Color {
    pub r: u8,
    pub g: u8,
    pub b: u8,
}

impl Color {
    pub const BLACK: Color = Color { r: 0, g: 0, b: 0 };
    pub const WHITE: Color = Color { r: 255, g: 255, b: 255 };
    pub const RED: Color = Color { r: 255, g: 0, b: 0 };
    pub const GREEN: Color = Color { r: 0, g: 255, b: 0 };

    pub const fn new(r: u8, g: u8, b: u8) -> Self {
        Self { r, g, b }
    }

    /// Parses `#rrggbb` or `rrggbb`; anything else becomes black.
    pub fn from_hex(s: &str) -> Self {
        let s = s.trim().trim_start_matches('#');
        if s.len() >= 6 {
            let p = |i: usize| u8::from_str_radix(&s[i..i + 2], 16).unwrap_or(0);
            Self::new(p(0), p(2), p(4))
        } else {
            Self::BLACK
        }
    }

    pub fn to_hex(self) -> String {
        format!("#{:02x}{:02x}{:02x}", self.r, self.g, self.b)
    }

    pub fn from_hsv(h: f64, s: f64, v: f64) -> Self {
        let c = v * s;
        let x = c * (1.0 - ((h / 60.0) % 2.0 - 1.0).abs());
        let m = v - c;
        let (r, g, b) = match h {
            h if (0.0..60.0).contains(&h) => (c, x, 0.0),
            h if (60.0..120.0).contains(&h) => (x, c, 0.0),
            h if (120.0..180.0).contains(&h) => (0.0, c, x),
            h if (180.0..240.0).contains(&h) => (0.0, x, c),
            h if (240.0..300.0).contains(&h) => (x, 0.0, c),
            _ => (c, 0.0, x),
        };
        let conv = |v: f64| ((v + m) * 255.0).round().clamp(0.0, 255.0) as u8;
        Self::new(conv(r), conv(g), conv(b))
    }

    pub fn scale(&self, factor: f64) -> Self {
        let conv = |v: u8| (v as f64 * factor).round().clamp(0.0, 255.0) as u8;
        Self::new(conv(self.r), conv(self.g), conv(self.b))
    }
}

#[derive(Debug, Clone)]
pub struct Mode {
    pub idx: u32,
    pub name: String,
    pub value: i32,
    pub flags: u32,
    pub speed_min: u32,
    pub speed_max: u32,
    pub brightness_min: u32,
    pub brightness_max: u32,
    pub colors_min: u32,
    pub colors_max: u32,
    pub speed: u32,
    pub brightness: u32,
    pub direction: u32,
    pub color_mode: u32,
    pub colors: Vec<Color>,
}

#[derive(Debug, Clone)]
pub struct Zone {
    #[allow(dead_code)]
    pub name: String,
    pub kind: u32,
    pub leds_count: u32,
    pub matrix_height: u32,
    pub matrix_width: u32,
    pub matrix_values: Vec<u32>,
}

#[derive(Debug, Clone)]
pub struct Led {
    pub idx: u32,
    pub name: String,
}

#[derive(Debug)]
pub struct Device {
    pub idx: u32,
    pub kind: u32,
    pub name: String,
    pub modes: Vec<Mode>,
    pub zones: Vec<Zone>,
    pub leds: Vec<Led>,
    pub enabled: AtomicBool,
}

impl Device {
    pub fn enabled(&self) -> bool {
        self.enabled.load(Ordering::SeqCst)
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// Binary helpers
// ---------------------------------------------------------------------------------------------------------------------

struct Reader<'a> {
    buf: &'a [u8],
    pos: usize,
}

impl<'a> Reader<'a> {
    fn new(buf: &'a [u8]) -> Self {
        Self { buf, pos: 0 }
    }

    fn take(&mut self, n: usize) -> Result<&'a [u8], String> {
        if self.pos + n > self.buf.len() {
            return Err("Unexpected end of OpenRGB packet".into());
        }
        let s = &self.buf[self.pos..self.pos + n];
        self.pos += n;
        Ok(s)
    }

    fn u16(&mut self) -> Result<u16, String> {
        Ok(u16::from_le_bytes(self.take(2)?.try_into().unwrap()))
    }

    fn u32(&mut self) -> Result<u32, String> {
        Ok(u32::from_le_bytes(self.take(4)?.try_into().unwrap()))
    }

    fn string(&mut self) -> Result<String, String> {
        let len = self.u16()? as usize;
        let raw = self.take(len)?;
        let end = raw.iter().position(|b| *b == 0).unwrap_or(raw.len());
        Ok(String::from_utf8_lossy(&raw[..end]).to_string())
    }

    fn color(&mut self) -> Result<Color, String> {
        let c = self.take(4)?;
        Ok(Color::new(c[0], c[1], c[2]))
    }
}

fn put_u16(out: &mut Vec<u8>, v: u16) {
    out.extend_from_slice(&v.to_le_bytes());
}

fn put_u32(out: &mut Vec<u8>, v: u32) {
    out.extend_from_slice(&v.to_le_bytes());
}

fn put_string(out: &mut Vec<u8>, s: &str) {
    put_u16(out, (s.len() + 1) as u16);
    out.extend_from_slice(s.as_bytes());
    out.push(0);
}

fn put_color(out: &mut Vec<u8>, c: &Color) {
    out.extend_from_slice(&[c.r, c.g, c.b, 0]);
}

fn parse_mode(r: &mut Reader, idx: u32) -> Result<Mode, String> {
    let name = r.string()?;
    let value = r.u32()? as i32;
    let flags = r.u32()?;
    let speed_min = r.u32()?;
    let speed_max = r.u32()?;
    let brightness_min = r.u32()?;
    let brightness_max = r.u32()?;
    let colors_min = r.u32()?;
    let colors_max = r.u32()?;
    let speed = r.u32()?;
    let brightness = r.u32()?;
    let direction = r.u32()?;
    let color_mode = r.u32()?;
    let count = r.u16()?;
    let colors = (0..count).map(|_| r.color()).collect::<Result<_, _>>()?;
    Ok(Mode {
        idx,
        name,
        value,
        flags,
        speed_min,
        speed_max,
        brightness_min,
        brightness_max,
        colors_min,
        colors_max,
        speed,
        brightness,
        direction,
        color_mode,
        colors,
    })
}

fn serialize_mode(out: &mut Vec<u8>, m: &Mode) {
    put_string(out, &m.name);
    put_u32(out, m.value as u32);
    put_u32(out, m.flags);
    put_u32(out, m.speed_min);
    put_u32(out, m.speed_max);
    put_u32(out, m.brightness_min);
    put_u32(out, m.brightness_max);
    put_u32(out, m.colors_min);
    put_u32(out, m.colors_max);
    put_u32(out, m.speed);
    put_u32(out, m.brightness);
    put_u32(out, m.direction);
    put_u32(out, m.color_mode);
    put_u16(out, m.colors.len() as u16);
    for c in &m.colors {
        put_color(out, c);
    }
}

fn parse_device(body: &[u8], idx: u32) -> Result<Device, String> {
    let mut r = Reader::new(body);
    let _data_size = r.u32()?;
    let kind = r.u32()?;
    let name = r.string()?;
    let _vendor = r.string()?;
    let _description = r.string()?;
    let _version = r.string()?;
    let _serial = r.string()?;
    let _location = r.string()?;
    let num_modes = r.u16()?;
    let _active_mode = r.u32()?;
    let modes = (0..num_modes as u32).map(|i| parse_mode(&mut r, i)).collect::<Result<_, _>>()?;

    let num_zones = r.u16()?;
    let mut zones = Vec::with_capacity(num_zones as usize);
    for _ in 0..num_zones {
        let name = r.string()?;
        let kind = r.u32()?;
        let _leds_min = r.u32()?;
        let _leds_max = r.u32()?;
        let leds_count = r.u32()?;
        let matrix_len = r.u16()?;
        let (mut matrix_height, mut matrix_width, mut matrix_values) = (0, 0, vec![]);
        if matrix_len > 0 {
            matrix_height = r.u32()?;
            matrix_width = r.u32()?;
            matrix_values = (0..matrix_height * matrix_width).map(|_| r.u32()).collect::<Result<_, _>>()?;
        }
        zones.push(Zone {
            name,
            kind,
            leds_count,
            matrix_height,
            matrix_width,
            matrix_values,
        });
    }

    let num_leds = r.u16()?;
    let mut leds = Vec::with_capacity(num_leds as usize);
    for i in 0..num_leds as u32 {
        let name = r.string()?;
        let _value = r.u32()?;
        leds.push(Led { idx: i, name });
    }

    Ok(Device {
        idx,
        kind,
        name,
        modes,
        zones,
        leds,
        enabled: AtomicBool::new(true),
    })
}

// ---------------------------------------------------------------------------------------------------------------------
// Connection
// ---------------------------------------------------------------------------------------------------------------------

pub struct OrgbConnection {
    stream: Mutex<TcpStream>,
    version: u32,
}

fn write_packet(stream: &mut TcpStream, device: u32, kind: u32, body: &[u8]) -> Result<(), String> {
    let mut pkt = Vec::with_capacity(16 + body.len());
    pkt.extend_from_slice(b"ORGB");
    put_u32(&mut pkt, device);
    put_u32(&mut pkt, kind);
    put_u32(&mut pkt, body.len() as u32);
    pkt.extend_from_slice(body);
    stream.write_all(&pkt).map_err(|e| e.to_string())
}

/// Reads packets until one of type `kind` arrives (notifications like DEVICE_LIST_UPDATED are skipped).
fn read_reply(stream: &mut TcpStream, kind: u32) -> Result<(u32, Vec<u8>), String> {
    loop {
        let mut header = [0u8; 16];
        stream.read_exact(&mut header).map_err(|e| e.to_string())?;
        if &header[0..4] != b"ORGB" {
            return Err("Invalid OpenRGB packet header".into());
        }
        let device = u32::from_le_bytes(header[4..8].try_into().unwrap());
        let pkt_kind = u32::from_le_bytes(header[8..12].try_into().unwrap());
        let size = u32::from_le_bytes(header[12..16].try_into().unwrap()) as usize;
        let mut body = vec![0u8; size];
        stream.read_exact(&mut body).map_err(|e| e.to_string())?;
        if pkt_kind == kind {
            return Ok((device, body));
        }
    }
}

impl OrgbConnection {
    pub fn connect(host: &str, port: u16, client_name: &str) -> Result<Self, String> {
        let mut stream = TcpStream::connect((host, port)).map_err(|e| e.to_string())?;
        stream.set_read_timeout(Some(Duration::from_secs(10))).map_err(|e| e.to_string())?;
        let _ = stream.set_nodelay(true);

        write_packet(&mut stream, 0, REQUEST_PROTOCOL_VERSION, &PROTOCOL_VERSION.to_le_bytes())?;
        let (_, body) = read_reply(&mut stream, REQUEST_PROTOCOL_VERSION)?;
        let server_version = Reader::new(&body).u32().unwrap_or(0);
        let version = PROTOCOL_VERSION.min(server_version);
        if version < 3 {
            return Err(format!("Unsupported OpenRGB protocol version {server_version}"));
        }

        let mut name = client_name.as_bytes().to_vec();
        name.push(0);
        write_packet(&mut stream, 0, SET_CLIENT_NAME, &name)?;

        Ok(Self {
            stream: Mutex::new(stream),
            version,
        })
    }

    pub fn request_devices(&self) -> Result<Vec<Device>, String> {
        let mut stream = self.stream.lock().unwrap();
        write_packet(&mut stream, 0, REQUEST_CONTROLLER_COUNT, &[])?;
        let (_, body) = read_reply(&mut stream, REQUEST_CONTROLLER_COUNT)?;
        let count = Reader::new(&body).u32()?;
        let mut devices = vec![];
        for idx in 0..count {
            write_packet(&mut stream, idx, REQUEST_CONTROLLER_DATA, &self.version.to_le_bytes())?;
            let (_, body) = read_reply(&mut stream, REQUEST_CONTROLLER_DATA)?;
            devices.push(parse_device(&body, idx)?);
        }
        Ok(devices)
    }

    pub fn change_mode(&self, device: &Device, mode: &Mode) -> Result<(), String> {
        let mut inner = vec![];
        put_u32(&mut inner, mode.idx);
        serialize_mode(&mut inner, mode);
        let mut body = vec![];
        put_u32(&mut body, (inner.len() + 4) as u32);
        body.extend_from_slice(&inner);
        write_packet(&mut self.stream.lock().unwrap(), device.idx, RGBCONTROLLER_UPDATEMODE, &body)
    }

    pub fn set_device_colors(&self, device: &Device, colors: &[Color]) -> Result<(), String> {
        let mut body = vec![];
        put_u32(&mut body, (4 + 2 + colors.len() * 4) as u32);
        put_u16(&mut body, colors.len() as u16);
        for c in colors {
            put_color(&mut body, c);
        }
        write_packet(&mut self.stream.lock().unwrap(), device.idx, RGBCONTROLLER_UPDATELEDS, &body)
    }

    pub fn set_device_color(&self, device: &Device, color: Color) -> Result<(), String> {
        self.set_device_colors(device, &vec![color; device.leds.len()])
    }

    pub fn disconnect(&self) {
        let _ = self.stream.lock().unwrap().shutdown(std::net::Shutdown::Both);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn hsv_and_hex() {
        assert_eq!(Color::from_hsv(0.0, 1.0, 1.0), Color::RED);
        assert_eq!(Color::from_hsv(120.0, 1.0, 1.0), Color::GREEN);
        assert_eq!(Color::from_hex("#00ff00"), Color::GREEN);
        assert_eq!(Color::RED.to_hex(), "#ff0000");
        assert_eq!(Color::WHITE.scale(0.5), Color::new(128, 128, 128));
    }

    #[test]
    fn device_parsing_roundtrip() {
        // Build a device body with one mode, one linear zone, two leds
        let mut b = vec![];
        put_u32(&mut b, 0); // data size (ignored)
        put_u32(&mut b, DEVICE_TYPE_KEYBOARD);
        for s in ["Keyboard", "ASUS", "desc", "1.0", "serial", "loc"] {
            put_string(&mut b, s);
        }
        put_u16(&mut b, 1);
        put_u32(&mut b, 0);
        let mode = Mode {
            idx: 0,
            name: "Direct".into(),
            value: 0,
            flags: 32,
            speed_min: 0,
            speed_max: 0,
            brightness_min: 0,
            brightness_max: 0,
            colors_min: 0,
            colors_max: 0,
            speed: 0,
            brightness: 0,
            direction: 0,
            color_mode: 1,
            colors: vec![Color::RED],
        };
        serialize_mode(&mut b, &mode);
        put_u16(&mut b, 1);
        put_string(&mut b, "Zone");
        for v in [1u32, 2, 2, 2] {
            put_u32(&mut b, v);
        }
        put_u16(&mut b, 0);
        put_u16(&mut b, 2);
        for name in ["Key: W", "Key: A"] {
            put_string(&mut b, name);
            put_u32(&mut b, 0);
        }
        put_u16(&mut b, 0);

        let dev = parse_device(&b, 3).unwrap();
        assert_eq!(dev.idx, 3);
        assert_eq!(dev.name, "Keyboard");
        assert_eq!(dev.modes[0].name, "Direct");
        assert_eq!(dev.modes[0].colors, vec![Color::RED]);
        assert_eq!(dev.zones[0].leds_count, 2);
        assert_eq!(dev.leds[1].name, "Key: A");
    }
}
