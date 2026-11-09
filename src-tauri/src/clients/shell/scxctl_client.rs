//! scxctl: sched_ext schedulers.

use std::{
    collections::BTreeMap,
    sync::{LazyLock, Mutex},
};

use log::{debug, error};

use crate::{clients::shell::CmdClient, utils::logger::tabbed};

struct ScxState {
    current: Option<String>,
    current_powersave: String,
}

pub struct ScxCtlClient {
    cmd: CmdClient,
    /// scheduler -> [performance args, powersave args]
    available_sched: BTreeMap<String, [String; 2]>,
    state: Mutex<ScxState>,
}

pub static SCXCTL: LazyLock<ScxCtlClient> = LazyLock::new(ScxCtlClient::new);

const LOG_SCX: &str = "ScxCtlClient";

impl ScxCtlClient {
    fn new() -> Self {
        let cmd = CmdClient::new("scxctl", LOG_SCX);
        let mut available_sched = BTreeMap::new();
        let mut current = None;
        if cmd.available() {
            let all = [
                ("bpfland", "", "-s 20000 -m powersave -I 100 -t 100"),
                ("cosmos", "-c 0 -p 0", "-m powersave -d -p 5000"),
                ("flash", "-m all", "-m powersave -I 10000 -t 10000 -s 10000 -S 1000"),
                ("lavd", "--performance", "--powersave"),
                ("p2dq", "--task-slice true -f --sched-mode performance", "--sched-mode efficiency"),
            ];
            let output = cmd.run("list", false, false).map(|r| r.stdout).unwrap_or_default();
            for (sched, perf, power) in all {
                if output.contains(&format!("\"{sched}\"")) {
                    available_sched.insert(sched.to_string(), [perf.to_string(), power.to_string()]);
                }
            }
            // "running <sched> in <mode> mode" / "no scx scheduler running"
            let get = cmd.run("get", false, false).map(|r| r.stdout).unwrap_or_default();
            if let Some(word) = get.split(' ').nth(1).map(|w| w.trim().to_lowercase())
                && word != "scx"
                && !word.is_empty()
            {
                current = Some(word);
            }
        }
        Self {
            cmd,
            available_sched,
            state: Mutex::new(ScxState {
                current,
                current_powersave: "performance".into(),
            }),
        }
    }

    pub fn available(&self) -> bool {
        self.cmd.available()
    }

    pub fn available_schedulers(&self) -> Vec<String> {
        self.available_sched.keys().cloned().collect()
    }

    pub fn start(&self, name: &str, powersave: bool) {
        let new_mode = if powersave { "powersave" } else { "performance" };
        let Some(args) = self.available_sched.get(name) else {
            error!(target: LOG_SCX, "Scheduler {name} not available");
            return;
        };
        let mut state = self.state.lock().unwrap();
        if state.current.as_deref() == Some(name) && state.current_powersave == new_mode {
            debug!(target: LOG_SCX, "Scheduler {name}-{new_mode} already applied");
            return;
        }
        let action = match &state.current {
            Some(cur) => {
                debug!(target: LOG_SCX, "Switching scheduler from {cur}-{} to {name}-{new_mode}", state.current_powersave);
                "switch"
            }
            None => {
                debug!(target: LOG_SCX, "Starting scheduler {name}-{new_mode}");
                "start"
            }
        };
        tabbed(|| {
            let res = self.cmd.run(
                &format!("{action} --sched {name} --args=\"{}\"", args[if powersave { 1 } else { 0 }]),
                false,
                true,
            );
            match res {
                Ok(_) => debug!(target: LOG_SCX, "Scheduler applied succesfully"),
                Err(e) => error!(target: LOG_SCX, "Error applying scheduler: {e}"),
            }
        });
        state.current = Some(name.to_string());
        state.current_powersave = new_mode.to_string();
    }

    pub fn stop(&self) {
        let mut state = self.state.lock().unwrap();
        let Some(cur) = state.current.clone() else {
            return;
        };
        debug!(target: LOG_SCX, "Stopping scheduler {cur}-{}", state.current_powersave);
        tabbed(|| {
            let _ = self.cmd.run("stop", false, true);
            debug!(target: LOG_SCX, "Scheduler stopped succesfully");
        });
        state.current = None;
        state.current_powersave = "performance".into();
    }
}
