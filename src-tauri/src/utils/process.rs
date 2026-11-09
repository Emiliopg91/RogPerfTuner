//! Process helpers. Signals and priorities go through the elevated shell.

use std::{collections::BTreeSet, fs};

use crate::utils::shell::Shell;

pub fn all_pids_of_hierarchy(pid: i32) -> BTreeSet<i32> {
    Shell::get()
        .run_elevated_command(
            &format!("pstree -p {pid} | grep -o '([0-9]\\+)' | grep -o '[0-9]\\+'"),
            false,
        )
        .map(|r| r.stdout.lines().filter_map(|l| l.trim().parse().ok()).collect())
        .unwrap_or_default()
}

fn join_pids(pids: &BTreeSet<i32>) -> String {
    pids.iter().map(|p| p.to_string()).collect::<Vec<_>>().join(" ")
}

pub fn send_signal_to_hierarchy(pid: i32, signal: i32) -> BTreeSet<i32> {
    let pids = all_pids_of_hierarchy(pid);
    if !pids.is_empty() {
        let _ = Shell::get()
            .run_elevated_command(&format!("kill -{signal} {} 2>/dev/null", join_pids(&pids)), false);
    }
    pids
}

pub fn renice_hierarchy(pid: i32, value: i32) -> BTreeSet<i32> {
    let pids = all_pids_of_hierarchy(pid);
    if !pids.is_empty() {
        let _ = Shell::get().run_elevated_command(&format!("renice {value} -p {}", join_pids(&pids)), false);
    }
    pids
}

pub fn ionice_hierarchy(pid: i32, class: u8, value: u8) -> BTreeSet<i32> {
    let pids = all_pids_of_hierarchy(pid);
    if !pids.is_empty() {
        let _ = Shell::get()
            .run_elevated_command(&format!("ionice -c{class} -n{value} -p {}", join_pids(&pids)), false);
    }
    pids
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
