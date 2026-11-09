//! Clients for the system, tools and services the application drives.
//! Holds the error type shared by every client.

pub mod dbus;
pub mod file;
pub mod lib;
pub mod shell;
pub mod tcp;
pub mod unix_socket;

use crate::utils::shell::ShellError;

#[derive(Debug, thiserror::Error)]
pub enum ClientError {
    #[error("File {0} doesn't exist")]
    Missing(String),
    #[error("Value {value} outside of range [{min},{max}]")]
    OutOfRange { value: i32, min: i32, max: i32 },
    #[error("Invalid value '{0}'")]
    Parse(String),
    #[error("Command {0} not available")]
    CommandUnavailable(String),
    #[error(transparent)]
    Shell(#[from] ShellError),
}

pub type ClientResult<T> = Result<T, ClientError>;
