//! Messages exchanged over the unix sockets (with the Decky plugin and between app instances).
//! Each message is a YAML document prefixed by its length as a big endian `u32`.

use std::io::{Read, Write};

use noyalib::Value;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct UnixMessage {
    #[serde(rename = "type", default)]
    pub kind: String,
    #[serde(default)]
    pub name: String,
    #[serde(default, deserialize_with = "deserialize_data")]
    pub data: Vec<Value>,
    #[serde(default)]
    pub id: String,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub error: Option<String>,
}

fn deserialize_data<'de, D: serde::Deserializer<'de>>(d: D) -> Result<Vec<Value>, D::Error> {
    Ok(Option::<Vec<Value>>::deserialize(d)?.unwrap_or_default())
}

impl UnixMessage {
    pub fn request(name: &str, data: Vec<Value>) -> Self {
        Self {
            kind: "REQUEST".into(),
            name: name.into(),
            data,
            id: crate::utils::random::uuid_v4(),
            error: None,
        }
    }

    pub fn to_yaml(&self) -> String {
        noyalib::to_string(self).unwrap_or_default()
    }

    pub fn from_yaml(s: &str) -> Result<Self, String> {
        noyalib::from_str(s).map_err(|e| e.to_string())
    }
}

/// Converts a data element into a string (numbers are formatted, maps/sequences re-serialized as YAML).
pub fn value_to_string(v: &Value) -> String {
    match v {
        Value::String(s) => s.clone(),
        Value::Null => String::new(),
        Value::Bool(b) => b.to_string(),
        Value::Number(_) => v
            .as_i64()
            .map(|n| n.to_string())
            .or_else(|| v.as_u64().map(|n| n.to_string()))
            .or_else(|| v.as_f64().map(|n| n.to_string()))
            .unwrap_or_default(),
        _ => noyalib::to_string(v).unwrap_or_default(),
    }
}

pub fn value_to_i64(v: &Value) -> Option<i64> {
    v.as_i64()
        .or_else(|| v.as_u64().map(|n| n as i64))
        .or_else(|| v.as_str().and_then(|s| s.trim().trim_end_matches('u').parse().ok()))
}

pub fn str_value(s: &str) -> Value {
    Value::String(s.to_string())
}

pub fn int_value(n: i64) -> Value {
    noyalib::to_value(&n).unwrap_or(Value::Null)
}

pub fn write_frame(stream: &mut impl Write, payload: &str) -> std::io::Result<()> {
    stream.write_all(&(payload.len() as u32).to_be_bytes())?;
    stream.write_all(payload.as_bytes())?;
    stream.flush()
}

/// Reads one length-prefixed frame. Returns `Ok(None)` when the peer closed the connection.
pub fn read_frame(stream: &mut impl Read) -> std::io::Result<Option<String>> {
    let mut len_buf = [0u8; 4];
    match stream.read_exact(&mut len_buf) {
        Ok(()) => {}
        Err(e) if e.kind() == std::io::ErrorKind::UnexpectedEof => return Ok(None),
        Err(e) => return Err(e),
    }
    let len = u32::from_be_bytes(len_buf) as usize;
    if len == 0 || len > 10 * 1024 * 1024 {
        return Err(std::io::Error::new(std::io::ErrorKind::InvalidData, format!("Invalid message length: {len}")));
    }
    let mut data = vec![0u8; len];
    stream.read_exact(&mut data)?;
    Ok(Some(String::from_utf8_lossy(&data).to_string()))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn message_roundtrip() {
        let msg = UnixMessage::request("gameConfig", vec![int_value(1091500), str_value("abc")]);
        let yaml = msg.to_yaml();
        let back = UnixMessage::from_yaml(&yaml).unwrap();
        assert_eq!(back.kind, "REQUEST");
        assert_eq!(back.name, "gameConfig");
        assert_eq!(value_to_i64(&back.data[0]), Some(1091500));
        assert_eq!(value_to_string(&back.data[1]), "abc");
        assert!(back.error.is_none());
    }

    #[test]
    fn parses_python_style_message() {
        let yaml = "type: EVENT\nname: launch_game\ndata:\n- 1091500\n- Cyberpunk 2077\n- 4242\nid: x\nerror: null\n";
        let msg = UnixMessage::from_yaml(yaml).unwrap();
        assert_eq!(msg.kind, "EVENT");
        assert_eq!(value_to_i64(&msg.data[2]), Some(4242));
        assert!(msg.error.is_none());
    }
}
