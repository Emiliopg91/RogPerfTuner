//! Nvidia throttle temperature. `None` when the attribute is not available.

use std::sync::LazyLock;

use crate::{clients::file::firmware::asus_armoury::{ArmouryClient, armoury}, features::features};

pub static NV_TEMP: LazyLock<Option<ArmouryClient>> = LazyLock::new(|| armoury(&features().nv_thermal));
