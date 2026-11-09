//! sysfs/procfs clients. Holds the shared file and glob clients: reads and writes go through the persistent
//! shell so they can be elevated when needed.

pub mod battery_charge_limit_client;
pub mod battery_status_client;
pub mod boost_control_client;
pub mod firmware;
pub mod intel_rapl_uj_client;
pub mod power_profile_client;
pub mod proc_modules_client;
pub mod scaling_governor_client;
pub mod sched_bore_client;
pub mod ssd_scheduler_client;

use std::path::Path;

use crate::{
    clients::{ClientError, ClientResult},
    utils::shell::{Shell, shell_quote},
};

#[derive(Debug, Clone)]
pub struct FileClient {
    path: String,
    sudo: bool,
    available: bool,
}

impl FileClient {
    pub fn new(path: &str, sudo: bool) -> Self {
        Self {
            path: path.to_string(),
            sudo,
            available: Path::new(path).exists(),
        }
    }

    pub fn available(&self) -> bool {
        self.available
    }

    pub fn read(&self) -> ClientResult<String> {
        if !self.available {
            return Err(ClientError::Missing(self.path.clone()));
        }
        read_file(&self.path, self.sudo)
    }

    pub fn write(&self, content: &str) -> ClientResult<()> {
        let cmd = format!("echo {} | tee {}", shell_quote(content), shell_quote(&self.path));
        if self.sudo {
            Shell::get().run_elevated_command(&cmd, true)?;
        } else {
            Shell::get().run_command(&cmd, true)?;
        }
        Ok(())
    }
}

/// Client over every file matching a glob (e.g. one per CPU core)
#[derive(Debug, Clone)]
pub struct GlobClient {
    pattern: String,
    paths: Vec<String>,
    sudo: bool,
}

impl GlobClient {
    pub fn new(pattern: &str, sudo: bool) -> Self {
        let paths = glob::glob(pattern)
            .map(|g| g.flatten().map(|p| p.display().to_string()).collect())
            .unwrap_or_default();
        Self {
            pattern: pattern.to_string(),
            paths,
            sudo,
        }
    }

    pub fn available(&self) -> bool {
        !self.paths.is_empty()
    }

    pub fn read(&self) -> ClientResult<Vec<String>> {
        if self.paths.is_empty() {
            return Err(ClientError::Missing(self.pattern.clone()));
        }
        self.paths.iter().map(|p| read_file(p, self.sudo)).collect()
    }

    pub fn write(&self, content: &str) -> ClientResult<()> {
        // The pattern is expanded by bash, writing every match at once
        let cmd = format!("echo {} | tee {}", shell_quote(content), self.pattern);
        if self.sudo {
            Shell::get().run_elevated_command(&cmd, true)?;
        } else {
            Shell::get().run_command(&cmd, true)?;
        }
        Ok(())
    }
}

/// Reads a file directly when possible (most sysfs attributes are world readable), falling back to the
/// shell (elevated when `sudo`) only when permission is denied.
fn read_file(path: &str, sudo: bool) -> ClientResult<String> {
    match std::fs::read_to_string(path) {
        Ok(content) => Ok(content),
        Err(e) if e.kind() == std::io::ErrorKind::PermissionDenied => {
            let cmd = format!("cat {}", shell_quote(path));
            let res = if sudo {
                Shell::get().run_elevated_command(&cmd, true)?
            } else {
                Shell::get().run_command(&cmd, true)?
            };
            Ok(res.stdout)
        }
        Err(_) => Err(ClientError::Missing(path.to_string())),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn reads_world_readable_files_without_shell() {
        let client = FileClient::new("/proc/sys/kernel/osrelease", true);
        let t0 = std::time::Instant::now();
        let value = client.read().unwrap();
        assert!(!value.trim().is_empty());
        // Far below a shell round trip
        assert!(t0.elapsed().as_millis() < 5);
    }
}
