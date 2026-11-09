//! Single instance lock.

use std::fs;

/// Kills a running instance registered in `lock_file`. Returns whether one was running.
pub fn kill_running_instance(lock_file: &str) -> bool {
    let mut running = false;
    if let Ok(content) = fs::read_to_string(lock_file) {
        if let Ok(pid) = content.trim().parse::<i32>()
            && pid > 0
            && pid != std::process::id() as i32
            && unsafe { libc::kill(pid, 0) } == 0
        {
            println!("Found instance with pid {pid}, killing...");
            unsafe { libc::kill(pid, libc::SIGKILL) };
            running = true;
        }
        let _ = fs::remove_file(lock_file);
    }
    running
}

pub fn acquire_single_instance(lock_file: &str) {
    kill_running_instance(lock_file);
    let _ = fs::write(lock_file, std::process::id().to_string());
}
