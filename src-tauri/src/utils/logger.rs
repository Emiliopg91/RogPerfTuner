//! Logger writing to stdout and to the log files with the format:
//! `[timestamp][LEVEL  ][Name                ] - <indent>message`.
//!
//! Every module logs with `target: "<Name>"`, and `add_tab()`/`rem_tab()` control the shared indentation.

use std::{
    env,
    fs::{self, File, OpenOptions},
    io::Write,
    path::Path,
    sync::{
        Mutex, OnceLock,
    },
    time::SystemTime,
};

use log::{Level, LevelFilter, Log, Metadata, Record};

thread_local! {
    /// Indentation level, per thread so concurrent initialization logs stay readable
    static TABS: std::cell::Cell<usize> = const { std::cell::Cell::new(0) };
}
static LOGGER: OnceLock<AppLogger> = OnceLock::new();

pub fn add_tab() {
    TABS.with(|t| t.set(t.get() + 1));
}

pub fn rem_tab() {
    TABS.with(|t| t.set(t.get().saturating_sub(1)));
}

/// Runs `f` with one extra indentation level.
pub fn tabbed<T>(f: impl FnOnce() -> T) -> T {
    add_tab();
    let res = f();
    rem_tab();
    res
}

struct AppLogger {
    level: LevelFilter,
    file: Option<Mutex<File>>,
}

impl Log for AppLogger {
    fn enabled(&self, metadata: &Metadata) -> bool {
        metadata.level() <= self.level
    }

    fn log(&self, record: &Record) {
        if !self.enabled(record.metadata()) {
            return;
        }
        // Third party crates (tauri, zbus...) only get through when they are warnings or worse
        let target = record.target();
        if target.contains("::") && record.level() > Level::Warn {
            return;
        }
        let level = match record.level() {
            Level::Error => "ERROR",
            Level::Warn => "WARN",
            Level::Info => "INFO",
            Level::Debug => "DEBUG",
            Level::Trace => "TRACE",
        };
        let mut name: String = target.chars().take(20).collect();
        while name.chars().count() < 20 {
            name.push(' ');
        }
        let line = format!(
            "[{}][{:<7}][{}] - {}{}\n",
            chrono::Local::now().format("%Y-%m-%d %H:%M:%S%.3f"),
            level,
            name,
            "  ".repeat(TABS.with(|t| t.get())),
            record.args()
        );
        print!("{line}");
        if let Some(file) = &self.file
            && let Ok(mut f) = file.lock()
        {
            let _ = f.write_all(line.as_bytes());
            let _ = f.flush();
        }
    }

    fn flush(&self) {}
}

/// Initializes logging to stdout and, when `file_name` is given, to `<dir>/<file_name>.log`
/// (rotating the previous file into `<dir>/old`, keeping the last 5).
pub fn initialize(file_name: Option<&str>, dir: Option<&str>) {
    let level = match env::var("RCC_LOG_LEVEL").map(|l| l.to_uppercase()).as_deref() {
        Ok("DEBUG") => LevelFilter::Debug,
        Ok("WARN") => LevelFilter::Warn,
        Ok("ERROR") | Ok("CRITICAL") => LevelFilter::Error,
        Ok("TRACE") => LevelFilter::Trace,
        _ if cfg!(debug_assertions) => LevelFilter::Debug,
        _ => LevelFilter::Info,
    };

    let file = match (file_name, dir) {
        (Some(name), Some(dir)) => {
            let old_dir = format!("{dir}/old");
            let _ = fs::create_dir_all(&old_dir);
            rotate_log(name, dir, &old_dir);
            OpenOptions::new()
                .create(true)
                .append(true)
                .open(format!("{dir}/{name}.log"))
                .ok()
                .map(Mutex::new)
        }
        _ => None,
    };

    let logger = LOGGER.get_or_init(|| AppLogger { level, file });
    if log::set_logger(logger).is_ok() {
        log::set_max_level(level);
    }
}

fn rotate_log(name: &str, dir: &str, old_dir: &str) {
    let current = Path::new(dir).join(format!("{name}.log"));
    if let Ok(meta) = fs::metadata(&current) {
        let stamp = meta
            .modified()
            .map(chrono::DateTime::<chrono::Local>::from)
            .unwrap_or_else(|_| chrono::Local::now())
            .format("%Y%m%d.%H%M%S%3f");
        let _ = fs::rename(&current, Path::new(old_dir).join(format!("{name}.{stamp}.log")));
    }

    let prefix = format!("{name}.");
    let mut olds: Vec<(SystemTime, std::path::PathBuf)> = fs::read_dir(old_dir)
        .map(|rd| {
            rd.flatten()
                .filter_map(|e| {
                    let fname = e.file_name().to_string_lossy().to_string();
                    if fname.starts_with(&prefix) && fname.ends_with(".log") {
                        let modified = e.metadata().and_then(|m| m.modified()).ok()?;
                        Some((modified, e.path()))
                    } else {
                        None
                    }
                })
                .collect()
        })
        .unwrap_or_default();
    olds.sort_by_key(|o| std::cmp::Reverse(o.0));
    for (_, path) in olds.into_iter().skip(5) {
        let _ = fs::remove_file(path);
    }
}
