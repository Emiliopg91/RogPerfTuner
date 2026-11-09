//! Dark background with WASD, numbers, volume and ROG keys highlighted.

use std::sync::LazyLock;

use regex::Regex;

use std::sync::Arc;

use crate::clients::tcp::open_rgb::{effects::EffectCtx, protocol::{Color, DEVICE_TYPE_KEYBOARD, DEVICE_TYPE_LAPTOP, Device}};

static GAMING_MATCHES: LazyLock<Vec<(Regex, Color)>> = LazyLock::new(|| {
    vec![
        (Regex::new(r"^Key: [1|2|3|4]$").unwrap(), Color::from_hex("#FFFF00")),
        (Regex::new(r"^Key: [W|A|S|D]$").unwrap(), Color::from_hex("#FF0000")),
        (Regex::new(r"^(?:Key: Media Volume [-+]|Key: Media Mute)$").unwrap(), Color::from_hex("#00FFFF")),
        (Regex::new(r"^(?:Key: Fan|Key: ROG)$").unwrap(), Color::from_hex("#FF4000")),
    ]
});

pub fn run(ctx: &EffectCtx, devices: &[Arc<Device>]) {
    let main = Color::from_hex("#000040");
    for dev in devices {
        let mut colors = vec![main; dev.leds.len()];
        if dev.kind == DEVICE_TYPE_KEYBOARD || dev.kind == DEVICE_TYPE_LAPTOP {
            for led in &dev.leds {
                if let Some((_, c)) = GAMING_MATCHES.iter().find(|(re, _)| re.is_match(&led.name)) {
                    colors[led.idx as usize] = *c;
                }
            }
        }
        ctx.set_colors(dev, &colors);
    }
    while ctx.running() {
        ctx.sleep(1.0);
    }
}
