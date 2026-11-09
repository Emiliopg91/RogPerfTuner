//! Command line tool clients. Holds the shared command wrapper.

pub mod asusctl_client;
pub mod flatpak_client;
pub mod scxctl_client;

use std::process::{Command, Stdio};

use log::error;

use crate::{
    clients::{ClientError, ClientResult},
    utils::shell::{CommandResult, Shell},
};

/// Wrapper over an external command, like `AbstractCmdClient`
#[derive(Debug, Clone)]
pub struct CmdClient {
    command: String,
    available: bool,
}

impl CmdClient {
    pub fn new(command: &str, log_name: &str) -> Self {
        let available = Command::new("which")
            .arg(command)
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status()
            .map(|s| s.success())
            .unwrap_or(false);
        if !available {
            error!(target: "CmdClient", "[{log_name}] Command {command} not available");
        }
        Self {
            command: command.to_string(),
            available,
        }
    }

    pub fn available(&self) -> bool {
        self.available
    }

    pub fn run(&self, args: &str, check: bool, sudo: bool) -> ClientResult<CommandResult> {
        if !self.available {
            return Err(ClientError::CommandUnavailable(self.command.clone()));
        }
        let cmd = format!("{} {}", self.command, args).trim().to_string();
        let res = if sudo {
            Shell::get().run_elevated_command(&cmd, check)?
        } else {
            Shell::get().run_command(&cmd, check)?
        };
        Ok(res)
    }
}
