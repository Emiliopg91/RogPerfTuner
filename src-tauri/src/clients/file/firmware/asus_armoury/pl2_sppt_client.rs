//! CPU short boost power limit (PL2). `None` when the attribute is not available.

use std::sync::LazyLock;

use crate::{clients::file::firmware::asus_armoury::{ArmouryClient, armoury}, features::features};

pub static PL2_SPPT: LazyLock<Option<ArmouryClient>> = LazyLock::new(|| armoury(&features().ppt_pl2_sppt));
