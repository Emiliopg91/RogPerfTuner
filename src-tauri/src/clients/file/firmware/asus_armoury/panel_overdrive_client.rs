//! Panel overdrive. `None` when the attribute is not available.

use std::sync::LazyLock;

use crate::{clients::file::firmware::asus_armoury::{ArmouryClient, armoury}, features::features};

pub static PANEL_OVERDRIVE: LazyLock<Option<ArmouryClient>> = LazyLock::new(|| armoury(&features().panel_od));
