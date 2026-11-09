//! File system helpers.

use std::{fs, io, os::unix::fs::PermissionsExt, path::Path};

pub fn exists(path: &str) -> bool {
    Path::new(path).exists()
}

pub fn write_file(path: &str, content: &str) -> io::Result<()> {
    if let Some(parent) = Path::new(path).parent() {
        fs::create_dir_all(parent)?;
    }
    fs::write(path, content)
}

pub fn remove_path(path: &str) -> io::Result<()> {
    let p = Path::new(path);
    if p.is_dir() {
        fs::remove_dir_all(p)
    } else if p.exists() {
        fs::remove_file(p)
    } else {
        Ok(())
    }
}

pub fn copy_path(src: &str, dst: &str) -> io::Result<()> {
    let s = Path::new(src);
    if s.is_dir() {
        fs::create_dir_all(dst)?;
        for entry in fs::read_dir(s)?.flatten() {
            let target = Path::new(dst).join(entry.file_name());
            copy_path(&entry.path().display().to_string(), &target.display().to_string())?;
        }
        Ok(())
    } else {
        if let Some(parent) = Path::new(dst).parent() {
            fs::create_dir_all(parent)?;
        }
        fs::copy(s, dst).map(|_| ())
    }
}

pub fn chmod_recursive(path: &str, mode: u32) -> io::Result<()> {
    let p = Path::new(path);
    fs::set_permissions(p, fs::Permissions::from_mode(mode))?;
    if p.is_dir() {
        for entry in fs::read_dir(p)?.flatten() {
            chmod_recursive(&entry.path().display().to_string(), mode)?;
        }
    }
    Ok(())
}

pub fn list_dir(path: &str) -> Vec<String> {
    fs::read_dir(path)
        .map(|rd| rd.flatten().map(|e| e.file_name().to_string_lossy().to_string()).collect())
        .unwrap_or_default()
}
