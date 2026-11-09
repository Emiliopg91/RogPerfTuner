//! Persistent bash sessions (regular and elevated through `sudo -kS bash`).
//!
//! Commands are written to the session's stdin followed by an `echo __END__$?` marker on stdout and an
//! `__END__` marker on stderr; both outputs are read until their marker shows up, which avoids spawning
//! (and authenticating) a process per command and waiting for stderr by guesswork.

use std::{
    collections::HashMap,
    io::{BufRead, BufReader, Read, Write},
    os::unix::process::CommandExt,
    process::{Child, ChildStdin, Command, Stdio},
    sync::{
        Arc, Condvar, Mutex, OnceLock,
        mpsc::{Receiver, channel},
    },
    thread,
    time::Duration,
};

use log::{debug, error, info};

use crate::utils::logger::tabbed;

const LOG: &str = "Shell";
const MARKER: &str = "__END__";

#[derive(Debug, Clone, Default)]
pub struct CommandResult {
    pub exit_code: i32,
    pub stdout: String,
    pub stderr: String,
}

#[derive(Debug, thiserror::Error)]
pub enum ShellError {
    #[error("Command '{cmd}' finished with code {code}. Error output: {stderr}")]
    Failed { cmd: String, code: i32, stderr: String },
    #[error("No elevated shell available")]
    NoElevated,
    #[error("Shell session closed")]
    Closed,
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
}

pub type ShellResult<T> = Result<T, ShellError>;

struct BashSession {
    child: Child,
    stdin: ChildStdin,
    stdout_rx: Receiver<String>,
    /// Accumulated stderr, notified on every chunk
    stderr_buf: Arc<(Mutex<String>, Condvar)>,
}

impl BashSession {
    fn start(args: &[&str], initial_input: Option<&str>) -> std::io::Result<Self> {
        let mut cmd = Command::new(args[0]);
        cmd.args(&args[1..])
            .env("LANG", "C")
            .env("LC_ALL", "C")
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .stderr(Stdio::piped());
        // Kill the session when this process dies
        unsafe {
            cmd.pre_exec(|| {
                libc::prctl(libc::PR_SET_PDEATHSIG, libc::SIGTERM);
                Ok(())
            });
        }
        let mut child = cmd.spawn()?;
        let mut stdin = child.stdin.take().unwrap();
        let stdout = child.stdout.take().unwrap();
        let mut stderr = child.stderr.take().unwrap();

        let (tx, stdout_rx) = channel::<String>();
        thread::spawn(move || {
            let mut reader = BufReader::new(stdout);
            let mut line = String::new();
            loop {
                line.clear();
                match reader.read_line(&mut line) {
                    Ok(0) | Err(_) => break,
                    Ok(_) => {
                        if tx.send(line.clone()).is_err() {
                            break;
                        }
                    }
                }
            }
        });

        let stderr_buf = Arc::new((Mutex::new(String::new()), Condvar::new()));
        let buf = stderr_buf.clone();
        thread::spawn(move || {
            let mut chunk = [0u8; 4096];
            loop {
                match stderr.read(&mut chunk) {
                    Ok(0) | Err(_) => break,
                    Ok(n) => {
                        buf.0.lock().unwrap().push_str(&String::from_utf8_lossy(&chunk[..n]));
                        buf.1.notify_all();
                    }
                }
            }
            // Wake up any waiter when the session dies
            buf.1.notify_all();
        });

        if let Some(input) = initial_input {
            stdin.write_all(input.as_bytes())?;
            stdin.flush()?;
        }

        Ok(Self {
            child,
            stdin,
            stdout_rx,
            stderr_buf,
        })
    }

    fn run(&mut self, cmd: &str) -> ShellResult<CommandResult> {
        self.stderr_buf.0.lock().unwrap().clear();
        let full = format!("{cmd}\necho {MARKER}$?\necho {MARKER} >&2\n");
        self.stdin.write_all(full.as_bytes())?;
        self.stdin.flush()?;

        let mut out = String::new();
        loop {
            let line = self.stdout_rx.recv().map_err(|_| ShellError::Closed)?;
            if let Some(pos) = line.find(MARKER) {
                out.push_str(&line[..pos]);
                let code = line[pos + MARKER.len()..].trim().parse::<i32>().unwrap_or(-1);
                let stderr = self.wait_stderr();
                return Ok(CommandResult {
                    exit_code: code,
                    stdout: out,
                    stderr,
                });
            }
            out.push_str(&line);
        }
    }
}

impl BashSession {
    /// Waits for the stderr marker, which comes after all the command's stderr output.
    fn wait_stderr(&self) -> String {
        let (lock, cvar) = &*self.stderr_buf;
        let guard = lock.lock().unwrap();
        let (mut guard, _) = cvar
            .wait_timeout_while(guard, Duration::from_secs(2), |buf| !buf.contains(MARKER))
            .unwrap();
        let stderr = match guard.find(MARKER) {
            Some(pos) => guard[..pos].to_string(),
            None => guard.clone(),
        };
        guard.clear();
        stderr
    }
}

impl Drop for BashSession {
    fn drop(&mut self) {
        let _ = self.child.kill();
    }
}

pub struct Shell {
    normal: Mutex<BashSession>,
    elevated: Mutex<Option<BashSession>>,
    which_cache: Mutex<HashMap<String, Vec<String>>>,
}

static SHELL: OnceLock<Shell> = OnceLock::new();

impl Shell {
    /// Initializes the shell, opening the elevated session when a password is provided.
    pub fn init(sudo_password: Option<&str>) -> &'static Shell {
        SHELL.get_or_init(|| {
            info!(target: LOG, "Initializing shells");
            let normal = tabbed(|| {
                info!(target: LOG, "Initializing standard");
                BashSession::start(&["bash"], None).expect("Couldn't start bash")
            });
            let shell = Shell {
                normal: Mutex::new(normal),
                elevated: Mutex::new(None),
                which_cache: Mutex::new(HashMap::new()),
            };
            if let Some(pass) = sudo_password.filter(|p| !p.is_empty()) {
                shell.set_password(pass);
            }
            shell
        })
    }

    /// Returns the shell, initializing an unprivileged one if needed.
    pub fn get() -> &'static Shell {
        Shell::init(None)
    }

    /// (Re)opens the elevated session with the given sudo password.
    pub fn set_password(&self, pass: &str) {
        tabbed(|| {
            info!(target: LOG, "Initializing admin");
            // SIGPIPE is ignored by the Rust runtime, so a dead shell surfaces as an IO/closed error
            match BashSession::start(&["sudo", "-kS", "bash"], Some(&format!("{pass}\n"))) {
                Ok(mut session) => match session.run("pwd") {
                    Ok(res) if res.exit_code == 0 => *self.elevated.lock().unwrap() = Some(session),
                    Ok(res) => error!(target: LOG, "Could not start elevated shell: {}", res.stderr.trim()),
                    Err(e) => error!(target: LOG, "Could not start elevated shell: {e}"),
                },
                Err(e) => error!(target: LOG, "Could not start elevated shell: {e}"),
            }
        });
    }

    /// Checks a sudo password by running `sudo -kS true` with it.
    pub fn validate_password(pass: &str) -> bool {
        let child = Command::new("sudo")
            .args(["-kS", "true"])
            .env("LANG", "C")
            .stdin(Stdio::piped())
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .spawn();
        match child {
            Ok(mut child) => {
                if let Some(mut stdin) = child.stdin.take() {
                    let _ = stdin.write_all(format!("{pass}\n").as_bytes());
                }
                child.wait().map(|s| s.success()).unwrap_or(false)
            }
            Err(_) => false,
        }
    }

    fn finish(cmd: &str, elevated: bool, res: CommandResult, check: bool) -> ShellResult<CommandResult> {
        debug!(target: LOG, "Command finished with code {}", res.exit_code);
        if res.exit_code != 0 {
            tabbed(|| debug!(target: LOG, "Error output: {}", res.stderr.trim()));
            if check {
                return Err(ShellError::Failed {
                    cmd: cmd.to_string(),
                    code: res.exit_code,
                    stderr: res.stderr,
                });
            }
        }
        let _ = elevated;
        Ok(res)
    }

    pub fn run_command(&self, cmd: &str, check: bool) -> ShellResult<CommandResult> {
        let mut session = self.normal.lock().unwrap();
        debug!(target: LOG, "Running command {cmd}");
        let res = session.run(cmd)?;
        Self::finish(cmd, false, res, check)
    }

    pub fn run_elevated_command(&self, cmd: &str, check: bool) -> ShellResult<CommandResult> {
        let mut guard = self.elevated.lock().unwrap();
        let session = guard.as_mut().ok_or(ShellError::NoElevated)?;
        debug!(target: LOG, "Running admin command {cmd}");
        let res = session.run(cmd)?;
        Self::finish(cmd, true, res, check)
    }

    pub fn which_all(&self, cmd: &str) -> Vec<String> {
        if let Some(cached) = self.which_cache.lock().unwrap().get(cmd) {
            return cached.clone();
        }
        match self.run_command(&format!("which -a {cmd}"), false) {
            Ok(res) if res.exit_code == 0 => {
                let all: Vec<String> = res
                    .stdout
                    .lines()
                    .map(|l| l.trim().to_string())
                    .filter(|l| !l.is_empty())
                    .collect();
                self.which_cache.lock().unwrap().insert(cmd.to_string(), all.clone());
                all
            }
            _ => vec![],
        }
    }

    pub fn which(&self, cmd: &str) -> Option<String> {
        self.which_all(cmd).into_iter().next()
    }

    /// Launches a command inside a terminal emulator (konsole), like `Shell::launch_in_terminal`.
    pub fn launch_in_terminal(&self, command: &str, detach: bool, hold: bool) -> std::io::Result<Child> {
        let mut cmd = Command::new("konsole");
        if detach {
            cmd.arg("--nofork");
        }
        if hold {
            cmd.arg("--hold");
        }
        cmd.arg("-e").arg(command);
        if detach {
            unsafe {
                cmd.pre_exec(|| {
                    libc::setsid();
                    Ok(())
                });
            }
        }
        cmd.spawn()
    }
}

/// Escapes a value to be safely embedded inside single quotes in a bash command.
pub fn shell_quote(value: &str) -> String {
    format!("'{}'", value.replace('\'', "'\\''"))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn splits_stdout_stderr_and_exit_code() {
        let mut s = BashSession::start(&["bash"], None).unwrap();
        let res = s.run("echo out; echo err >&2; false").unwrap();
        assert_eq!(res.stdout, "out\n");
        assert_eq!(res.stderr, "err\n");
        assert_eq!(res.exit_code, 1);
        // stderr of one command never leaks into the next
        let res = s.run("printf 'no-newline'").unwrap();
        assert_eq!(res.stdout, "no-newline");
        assert_eq!(res.stderr, "");
        assert_eq!(res.exit_code, 0);
    }

    #[test]
    fn command_latency() {
        let mut s = BashSession::start(&["bash"], None).unwrap();
        let t0 = std::time::Instant::now();
        for _ in 0..200 {
            s.run("true").unwrap();
        }
        let per_cmd = t0.elapsed().as_micros() / 200;
        println!("{per_cmd} µs per command");
        assert!(per_cmd < 2000, "{per_cmd} µs per command");
    }
}
