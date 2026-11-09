//! Command line options. Returns `None` when the GUI must be started.

use std::{env, fs::OpenOptions, os::unix::io::AsRawFd, os::unix::process::CommandExt, process::Command};

use log::{error, info, warn};

use crate::{
    clients::{shell::flatpak_client::FLATPAK, unix_socket::rog_perf_tuner_client as app_client},
    utils::{self, constants, logger, shell::Shell},
};

struct CliOption {
    long: &'static str,
    short: Option<&'static str>,
    description: Option<&'static str>,
}

const OPTIONS: &[(&str, &[CliOption])] = &[
    (
        "Performance Control",
        &[CliOption { long: "--performance", short: Some("-p"), description: Some("    Switch to next performance profile") }],
    ),
    (
        "RGB lightning control",
        &[
            CliOption { long: "--effect", short: Some("-e"), description: Some("         Switch to next lighting effect") },
            CliOption { long: "--inc-brightness", short: Some("-i"), description: Some("  Increase keyboard brightness") },
            CliOption { long: "--dec-brightness", short: Some("-d"), description: Some("  Decrease keyboard brightness") },
        ],
    ),
    (
        "Application",
        &[
            CliOption { long: "--show", short: Some("-s"), description: Some("           Show GUI") },
            CliOption { long: "--kill", short: Some("-k"), description: Some("           Kill existing instance") },
            CliOption { long: "--dev-mode", short: Some("-m"), description: Some("       Run in dev mode with bug report generation") },
            CliOption { long: "--version", short: Some("-v"), description: Some("        Show version information") },
            CliOption { long: "--help", short: Some("-h"), description: Some("           Show this help message") },
        ],
    ),
];

fn print_help(argv0: &str) {
    println!("Usage: {argv0} [option]");
    for (group, opts) in OPTIONS {
        println!("  {group}");
        for opt in *opts {
            let mut line = opt.long.to_string();
            if let Some(s) = opt.short {
                line = format!("{s}, {line}");
            }
            if let Some(d) = opt.description {
                line.push_str(d);
            }
            println!("    {line}");
        }
    }
}

fn print_result(res: Result<String, String>) -> i32 {
    match res {
        Ok(v) => {
            println!("{v}");
            0
        }
        Err(e) => {
            eprintln!("{e}");
            1
        }
    }
}

/// Handles the command line. Returns the exit code, or `None` to start the GUI.
pub fn handle(args: &[String]) -> Option<i32> {
    let argv0 = args.first().map(|s| s.as_str()).unwrap_or(constants::EXEC_NAME);
    let arg = args.get(1)?;
    let rest: Vec<String> = args.iter().skip(2).cloned().collect();
    Some(match arg.as_str() {
        "-v" | "--version" => {
            println!("{} v{}", constants::APP_NAME, constants::APP_VERSION);
            println!("Decky plugin v{}", constants::PLUGIN_VERSION);
            0
        }
        "-p" | "--performance" => {
            logger::initialize(None, None);
            print_result(app_client::next_profile())
        }
        "-e" | "--effect" => {
            logger::initialize(None, None);
            print_result(app_client::next_effect())
        }
        "-i" | "--inc-brightness" => {
            logger::initialize(None, None);
            print_result(app_client::increase_brightness())
        }
        "-d" | "--dec-brightness" => {
            logger::initialize(None, None);
            print_result(app_client::decrease_brightness())
        }
        "-k" | "--kill" => {
            if utils::single_instance::kill_running_instance(&constants::LOCK_FILE) {
                0
            } else {
                1
            }
        }
        "-s" | "--show" => {
            logger::initialize(None, None);
            match app_client::show_gui() {
                Ok(()) => 0,
                Err(e) => {
                    eprintln!("{e}");
                    1
                }
            }
        }
        "-m" | "--dev-mode" => run_dev_mode(),
        "-f" | "--flatpak" => run_flatpak_wrapping(argv0, &rest),
        "-r" | "--run" => run_steam_wrapping(&rest),
        "--completion" => {
            let all: Vec<&str> = OPTIONS.iter().flat_map(|(_, o)| o.iter().map(|o| o.long)).collect();
            println!("{}", all.join(" "));
            0
        }
        "-h" | "--help" => {
            print_help(argv0);
            0
        }
        other => {
            eprintln!("Invalid argument '{other}'");
            print_help(argv0);
            1
        }
    })
}

fn run_dev_mode() -> i32 {
    utils::single_instance::acquire_single_instance(&constants::LOCK_FILE);
    logger::initialize(Some(constants::LOG_FILE_NAME), Some(&constants::LOG_DIR));
    match Shell::get().launch_in_terminal(&constants::DEV_MODE_FILE, true, false) {
        Ok(mut child) => child.wait().map(|s| s.code().unwrap_or(0)).unwrap_or(1),
        Err(e) => {
            error!(target: "Main", "Couldn't launch dev mode: {e}");
            1
        }
    }
}

/// Wraps `flatpak` so the game environment reaches flatpak apps.
fn run_flatpak_wrapping(argv0: &str, args: &[String]) -> i32 {
    logger::initialize(None, None);
    println!(">>> Running flatpak wrapper");

    let exe_dir = std::path::Path::new(argv0)
        .parent()
        .map(|p| p.display().to_string())
        .unwrap_or_default();
    let path = env::var("PATH").unwrap_or_default().replacen(&format!("{exe_dir}:"), "", 1);
    unsafe { env::set_var("PATH", &path) };

    let mut command = match env::var("ORIG_FLATPAK_BIN") {
        Ok(bin) => bin,
        Err(_) => match Shell::get().which("flatpak") {
            Some(bin) => bin,
            None => {
                eprintln!("Command {argv0} not found");
                return 127;
            }
        },
    };
    println!("flatpak -> {command}");

    let mut mangohud = false;
    if args.iter().any(|a| a == "run")
        && let Ok(ofe) = env::var("OVERRIDE_FLATPAK_ENV")
    {
        for token in ofe.split(';').filter(|t| !t.is_empty()) {
            if let Ok(val) = env::var(token) {
                command.push_str(&format!(" --env={token}={val}"));
                if token == "MANGOHUD" {
                    mangohud = true;
                }
            }
        }
    }
    if mangohud && !FLATPAK.check_installed(constants::FLATPAK_MANGOHUD, false) {
        println!("MangoHud flatpak installation missing, attempting installation");
        FLATPAK.install(constants::FLATPAK_MANGOHUD, false);
        FLATPAK.override_perm(constants::FLATPAK_MANGOHUD_OVERRIDE, false);
        FLATPAK.override_perm(constants::FLATPAK_MANGOHUD_OVERRIDE, true);
    }
    for a in args {
        command.push(' ');
        command.push_str(a);
    }
    println!(">>> Command:\n  {command}");
    Command::new("sh")
        .args(["-c", &command])
        .status()
        .map(|s| s.code().unwrap_or(1))
        .unwrap_or(1)
}

/// Steam launch wrapper: applies the game configuration and replaces this process.
fn run_steam_wrapping(args: &[String]) -> i32 {
    logger::initialize(Some(constants::LOG_RUNNER_FILE_NAME), Some(&constants::LOG_DIR));
    const LOG: &str = "Runner";
    if args.is_empty() {
        error!(target: LOG, "Error: no command provided");
        return 1;
    }

    let path = format!("{}:{}", *constants::BIN_STEAM_DIR, env::var("PATH").unwrap_or_default());
    info!(target: LOG, "New PATH: {path}");
    unsafe { env::set_var("PATH", &path) };

    info!(target: LOG, "===== Started wrapping =====");
    info!(target: LOG, ">>> Environment:");
    logger::tabbed(|| {
        for (k, v) in env::vars() {
            info!(target: LOG, "{k}={v}");
        }
    });
    info!(target: LOG, ">>> Command:");
    logger::tabbed(|| info!(target: LOG, "{}", args.join(" ")));

    let shell = Shell::get();
    let Some(resolved) = shell.which_all(&args[0]).into_iter().next() else {
        error!(target: LOG, "Command {} not found", args[0]);
        return 127;
    };
    info!(target: LOG, "{} -> {resolved}", args[0]);
    let mut command = vec![resolved];
    command.extend(args.iter().skip(1).cloned());

    let mut wrappers = vec![];
    let mut parameters = String::new();
    match env::var("SteamGameId") {
        Ok(steam_id) => match app_client::game_config(&steam_id) {
            Ok(cfg) => {
                for (k, v) in &cfg.environment {
                    unsafe { env::set_var(k, v) };
                }
                if !cfg.environment.is_empty() {
                    let keys: Vec<&str> = cfg.environment.keys().map(|k| k.as_str()).collect();
                    unsafe { env::set_var("OVERRIDE_FLATPAK_ENV", keys.join(";")) };
                }
                wrappers = cfg.wrappers;
                parameters = cfg.parameters;
            }
            Err(e) => error!(target: LOG, "Error requesting configuration {e}"),
        },
        Err(_) => warn!(target: LOG, "No AppId provided"),
    }

    let flatpaks = shell.which_all("flatpak");
    if flatpaks.len() > 1 {
        unsafe { env::set_var("ORIG_FLATPAK_BIN", flatpaks.last().unwrap().trim()) };
        // The launched binary is the argument after `--`, or the first one
        let bin = (1..=args.len())
            .rev()
            .find(|i| *i == 1 || args[*i - 2] == "--")
            .map(|i| args[i - 1].clone());
        if let Some(bin) = bin.filter(|b| utils::files::exists(b))
            && let Ok(mut content) = std::fs::read_to_string(&bin)
        {
            let _ = utils::files::copy_path(&bin, &format!("{bin}.bk"));
            for wr in &flatpaks {
                content = content.replace(wr.as_str(), "flatpak");
            }
            let _ = std::fs::write(&bin, content);
        }
    }

    let mut final_args: Vec<String> = wrappers;
    final_args.extend(command);
    final_args.extend(parameters.split_whitespace().map(|s| s.to_string()));
    info!(target: LOG, ">>> Replacing current process with: {}", final_args.join(" "));

    let log_path = format!("{}/{}.log", *constants::LOG_DIR, constants::LOG_RUNNER_FILE_NAME);
    match OpenOptions::new().create(true).append(true).open(&log_path) {
        Ok(f) => unsafe {
            libc::dup2(f.as_raw_fd(), libc::STDOUT_FILENO);
            libc::dup2(f.as_raw_fd(), libc::STDERR_FILENO);
        },
        Err(e) => {
            error!(target: LOG, "Cannot open log file: {e}");
            return e.raw_os_error().unwrap_or(1);
        }
    }
    let err = Command::new(&final_args[0]).args(&final_args[1..]).exec();
    error!(target: LOG, "Error on execvp: {err}");
    err.raw_os_error().unwrap_or(1)
}
