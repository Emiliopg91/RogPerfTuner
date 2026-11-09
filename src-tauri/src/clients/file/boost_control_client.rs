//! CPU boost.

use crate::{
    clients::{ClientResult, file::FileClient},
    features::features,
};

pub fn set_boost(enabled: bool) -> ClientResult<()> {
    match &features().boost_control {
        Some((file, on, off)) => FileClient::new(file, true).write(if enabled { on } else { off }),
        None => Ok(()),
    }
}
