//! CPU fast boost power limit (PL3). `None` when the attribute is not available.

use std::sync::LazyLock;

use crate::{clients::file::firmware::asus_armoury::{ArmouryClient, armoury}, features::features};

pub static PL3_FPPT: LazyLock<Option<ArmouryClient>> = LazyLock::new(|| armoury(&features().ppt_pl3_fppt));
