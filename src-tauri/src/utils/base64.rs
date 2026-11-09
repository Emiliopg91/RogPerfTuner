//! Base64 encoding, used to send game artwork to the webview.

use std::{
    collections::HashMap,
    fs,
    sync::{LazyLock, Mutex},
    time::SystemTime,
};

/// Encoded images by path, with the modification time they were read at
static IMAGE_CACHE: LazyLock<Mutex<HashMap<String, (SystemTime, String)>>> = LazyLock::new(Default::default);

pub fn base64_encode(data: &[u8]) -> String {
    const TABLE: &[u8; 64] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    let mut out = String::with_capacity(data.len().div_ceil(3) * 4);
    for chunk in data.chunks(3) {
        let b = [chunk[0], *chunk.get(1).unwrap_or(&0), *chunk.get(2).unwrap_or(&0)];
        let n = ((b[0] as u32) << 16) | ((b[1] as u32) << 8) | b[2] as u32;
        out.push(TABLE[(n >> 18) as usize & 63] as char);
        out.push(TABLE[(n >> 12) as usize & 63] as char);
        out.push(if chunk.len() > 1 { TABLE[(n >> 6) as usize & 63] as char } else { '=' });
        out.push(if chunk.len() > 2 { TABLE[n as usize & 63] as char } else { '=' });
    }
    out
}

/// Reads an image into a `data:` URL, cached until the file changes (the games list asks for every icon on each
/// refresh).
pub fn image_data_url(path: &str) -> Option<String> {
    let mtime = fs::metadata(path).and_then(|m| m.modified()).ok()?;
    if let Some((cached_mtime, url)) = IMAGE_CACHE.lock().unwrap().get(path)
        && *cached_mtime == mtime
    {
        return Some(url.clone());
    }
    let data = fs::read(path).ok()?;
    let mime = if path.ends_with(".png") { "image/png" } else { "image/jpeg" };
    let url = format!("data:{mime};base64,{}", base64_encode(&data));
    IMAGE_CACHE.lock().unwrap().insert(path.to_string(), (mtime, url.clone()));
    Some(url)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn base64_matches_reference() {
        assert_eq!(base64_encode(b"hello"), "aGVsbG8=");
        assert_eq!(base64_encode(b"hi!"), "aGkh");
        assert_eq!(base64_encode(b"a"), "YQ==");
    }
}
