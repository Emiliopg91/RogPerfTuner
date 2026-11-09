//! CPU sustained power limit (PL1). `None` when the attribute is not available.

use std::sync::LazyLock;

use crate::{clients::file::firmware::asus_armoury::{ArmouryClient, armoury}, features::features};

pub static PL1_SPL: LazyLock<Option<ArmouryClient>> = LazyLock::new(|| armoury(&features().ppt_pl1_spl));
