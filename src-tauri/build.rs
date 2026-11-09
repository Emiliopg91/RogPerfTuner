use std::{env, fs, path::PathBuf, process::Command};

fn main() {
    tauri_build::build();
    export_versions();
}

/// Exposes `RPT_APP_VERSION` (the crate version, plus the branch name outside `main`) and
/// `RPT_PLUGIN_VERSION` (bundled Decky plugin) at compile time.
fn export_versions() {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let repo_dir = manifest_dir.parent().unwrap().to_path_buf();

    let plugin_file = repo_dir
        .join("submodules")
        .join("RccDeckyCompanion")
        .join("package.json");
    println!("cargo:rerun-if-changed={}", plugin_file.display());

    let mut version = env::var("CARGO_PKG_VERSION").unwrap();

    if let Ok(output) = Command::new("git")
        .args(["rev-parse", "--abbrev-ref", "HEAD"])
        .current_dir(&repo_dir)
        .output()
    {
        let branch = String::from_utf8_lossy(&output.stdout).trim().to_string();
        if !branch.is_empty() && branch != "main" && branch != "HEAD" {
            version = format!("{version}-{branch}");
        }
    }

    let plugin_version = fs::read_to_string(&plugin_file)
        .ok()
        .and_then(|content| serde_json_lite_version(&content))
        .unwrap_or_else(|| "0.0.0".to_string());

    println!("cargo:rustc-env=RPT_APP_VERSION={version}");
    println!("cargo:rustc-env=RPT_PLUGIN_VERSION={plugin_version}");
}

/// Extracts the top-level `"version"` value from a package.json without pulling serde into the build script.
fn serde_json_lite_version(content: &str) -> Option<String> {
    let idx = content.find("\"version\"")?;
    let rest = &content[idx + "\"version\"".len()..];
    let start = rest.find('"')? + 1;
    let end = rest[start..].find('"')? + start;
    Some(rest[start..end].trim().to_string())
}
