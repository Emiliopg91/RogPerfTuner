//! Intel RAPL energy counter, made readable for MangoHud.

use crate::{
    features::features,
    utils::shell::{Shell, shell_quote},
};

pub fn enable_intel_rapl_read() {
    if let Some(path) = &features().intel_rapl_uj {
        let _ = Shell::get().run_elevated_command(&format!("chmod o+r {}", shell_quote(path)), false);
    }
}
