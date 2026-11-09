//! BORE scheduler toggle.

use std::sync::LazyLock;

use crate::clients::file::FileClient;

pub static SCHED_BORE: LazyLock<FileClient> = LazyLock::new(|| FileClient::new("/proc/sys/kernel/sched_bore", true));
