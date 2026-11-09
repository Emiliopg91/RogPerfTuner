//! flatpak.

use std::sync::LazyLock;

use crate::clients::shell::CmdClient;

pub struct FlatpakClient {
    cmd: CmdClient,
}

pub static FLATPAK: LazyLock<FlatpakClient> = LazyLock::new(|| FlatpakClient {
    cmd: CmdClient::new("flatpak", "FlatpakClient"),
});

impl FlatpakClient {
    fn scoped(&self, action: &str, name: &str, userland: bool) -> bool {
        let scope = if userland { "--user" } else { "--system" };
        self.cmd
            .run(&format!("{action} {scope} {name}"), false, false)
            .map(|r| r.exit_code == 0)
            .unwrap_or(false)
    }

    pub fn check_installed(&self, name: &str, userland: bool) -> bool {
        self.scoped("info", name, userland)
    }

    pub fn install(&self, name: &str, userland: bool) -> bool {
        self.scoped("install -y", name, userland)
    }

    pub fn override_perm(&self, name: &str, userland: bool) -> bool {
        self.scoped("override", name, userland)
    }
}
