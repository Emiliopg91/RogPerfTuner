//! Process helpers. The process tree is read from `/proc`; priorities need the elevated shell.

use std::{
    collections::{BTreeSet, HashMap},
    fs,
};

use crate::utils::shell::Shell;

/// Parent PID from `/proc/<pid>/stat` (the process name may contain spaces and parentheses).
fn parent_pid(stat: &str) -> Option<i32> {
    let rest = &stat[stat.rfind(')')? + 1..];
    rest.split_whitespace().nth(1)?.parse().ok()
}

/// Every process descending from `pid` (included) plus all their threads, like `pstree -p`:
/// priorities are per thread in Linux, so every thread must be reniced.
pub fn all_pids_of_hierarchy(pid: i32) -> BTreeSet<i32> {
    let mut children: HashMap<i32, Vec<i32>> = HashMap::new();
    for entry in fs::read_dir("/proc").into_iter().flatten().flatten() {
        if let Some(child) = entry.file_name().to_str().and_then(|n| n.parse::<i32>().ok())
            && let Some(ppid) = fs::read_to_string(format!("/proc/{child}/stat")).ok().as_deref().and_then(parent_pid)
        {
            children.entry(ppid).or_default().push(child);
        }
    }

    let mut result = BTreeSet::new();
    let mut pending = vec![pid];
    while let Some(p) = pending.pop() {
        let tasks: Vec<i32> = fs::read_dir(format!("/proc/{p}/task"))
            .into_iter()
            .flatten()
            .flatten()
            .filter_map(|e| e.file_name().to_str().and_then(|n| n.parse().ok()))
            .collect();
        if tasks.is_empty() {
            // Already gone
            continue;
        }
        result.insert(p);
        result.extend(tasks);
        if let Some(c) = children.get(&p) {
            pending.extend(c);
        }
    }
    result
}

fn join_pids(pids: &BTreeSet<i32>) -> String {
    pids.iter().map(|p| p.to_string()).collect::<Vec<_>>().join(" ")
}

/// Sends `signal` to the whole hierarchy. Game processes belong to the user, so this is a direct `kill()`;
/// the elevated shell is only used for the ones that refuse it.
pub fn send_signal_to_hierarchy(pid: i32, signal: i32) -> BTreeSet<i32> {
    let pids = all_pids_of_hierarchy(pid);
    let denied: BTreeSet<i32> = pids
        .iter()
        .copied()
        .filter(|p| unsafe { libc::kill(*p, signal) } != 0 && std::io::Error::last_os_error().raw_os_error() == Some(libc::EPERM))
        .collect();
    if !denied.is_empty() {
        let _ = Shell::get().run_elevated_command(&format!("kill -{signal} {} 2>/dev/null", join_pids(&denied)), false);
    }
    pids
}

/// Raising priority (negative nice) needs root.
pub fn renice_pids(pids: &BTreeSet<i32>, value: i32) {
    if !pids.is_empty() {
        let _ = Shell::get().run_elevated_command(&format!("renice {value} -p {}", join_pids(pids)), false);
    }
}

pub fn ionice_pids(pids: &BTreeSet<i32>, class: u8, value: u8) {
    if !pids.is_empty() {
        let _ = Shell::get().run_elevated_command(&format!("ionice -c{class} -n{value} -p {}", join_pids(pids)), false);
    }
}

pub fn cmdline(pid: i32) -> Vec<String> {
    fs::read(format!("/proc/{pid}/cmdline"))
        .map(|raw| {
            raw.split(|b| *b == 0)
                .filter(|s| !s.is_empty())
                .map(|s| String::from_utf8_lossy(s).to_string())
                .collect()
        })
        .unwrap_or_default()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_parent_pid_with_tricky_names() {
        assert_eq!(parent_pid("1234 (Web Content) S 42 1234 1234 0"), Some(42));
        assert_eq!(parent_pid("99 (a) b (c)) R 7 99"), Some(7));
    }

    #[test]
    fn hierarchy_includes_children_and_threads() {
        let mut child = std::process::Command::new("sleep").arg("5").spawn().unwrap();
        let me = std::process::id() as i32;
        let pids = all_pids_of_hierarchy(me);
        let _ = child.kill();
        let _ = child.wait();
        assert!(pids.contains(&me));
        assert!(pids.contains(&(child.id() as i32)), "child missing: {pids:?}");
        // The test harness runs on several threads
        assert!(pids.len() > 2);
    }
}
