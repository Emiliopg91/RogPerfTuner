//! NVMe IO schedulers.

use std::sync::LazyLock;

use crate::clients::{ClientResult, file::GlobClient};

/// NVMe IO schedulers available on every drive
pub struct SsdSchedulerClient {
    glob: GlobClient,
    schedulers: Vec<String>,
}

impl SsdSchedulerClient {
    fn new() -> Self {
        let glob = GlobClient::new("/sys/block/nvme*n*/queue/scheduler", true);
        let mut schedulers: Vec<String> = vec![];
        if glob.available()
            && let Ok(lines) = glob.read()
        {
            for (i, line) in lines.iter().enumerate() {
                let tmp: Vec<String> = line
                    .trim()
                    .replace(['[', ']'], "")
                    .split(' ')
                    .filter(|s| !s.is_empty())
                    .map(|s| s.to_string())
                    .collect();
                if i == 0 {
                    schedulers = tmp;
                } else {
                    schedulers.retain(|s| tmp.contains(s));
                }
            }
        }
        Self { glob, schedulers }
    }

    pub fn available(&self) -> bool {
        self.glob.available()
    }

    pub fn available_schedulers(&self) -> Vec<String> {
        self.schedulers.clone()
    }

    pub fn set_scheduler(&self, sched: &str) -> ClientResult<()> {
        self.glob.write(sched)
    }
}

pub static SSD_SCHEDULER: LazyLock<SsdSchedulerClient> = LazyLock::new(SsdSchedulerClient::new);
