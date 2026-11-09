//! CPU frequency governor of every core.

use crate::{
    clients::{ClientResult, file::GlobClient},
    features::features,
    models::performance::CpuGovernor,
};

pub fn set_governor(governor: CpuGovernor) -> ClientResult<()> {
    match &features().scaling_governor {
        Some(pattern) => GlobClient::new(pattern, true).write(governor.as_str()),
        None => Ok(()),
    }
}
