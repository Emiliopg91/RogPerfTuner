//! Boot sound. `None` when the attribute is not available.

use std::sync::LazyLock;

use crate::{clients::file::firmware::asus_armoury::{ArmouryClient, armoury}, features::features};

pub static BOOT_SOUND: LazyLock<Option<ArmouryClient>> = LazyLock::new(|| armoury(&features().boot_sound));
