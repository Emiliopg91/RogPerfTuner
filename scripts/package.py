import os
import shutil
import subprocess

workspace_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
build_dir = os.path.join(workspace_dir, "build")
output_dir = os.path.join(workspace_dir, "out")
dist_dir = os.path.join(workspace_dir, "dist")


def copy_icon():
    """Copy application icon"""
    src_icon_file_path = os.path.join(workspace_dir, "assets", "icons", "rog-logo.svg")
    dst_icon_file_path = os.path.join(output_dir, "icon.svg")

    print(f"Copying icon file to {dst_icon_file_path}...")
    shutil.copy2(src_icon_file_path, dst_icon_file_path)


def generate_desktop():
    """Generate .desktop file"""
    src_file_path = os.path.join(workspace_dir, "RogControlCenter.desktop")
    dst_file_path = os.path.join(output_dir, "RogControlCenter.desktop")

    print(f"Copying .desktop file to {dst_file_path}...")
    shutil.copy2(src_file_path, dst_file_path)
    os.chmod(dst_file_path, 0o755)


def generate_apprun():
    """Generate AppRun file"""
    content = """#!/bin/bash
set -e

if [ ! -z "$DEBUG" ] ; then
  env
  set -x
fi

THIS="$0"
# http://stackoverflow.com/questions/3190818/
args=("$@")
NUMBER_OF_ARGS="$#"

if [ -z "$APPDIR" ] ; then
  # Find the AppDir. It is the directory that contains AppRun.
  # This assumes that this script resides inside the AppDir or a subdirectory.
  # If this script is run inside an AppImage, then the AppImage runtime likely has already set $APPDIR
  path="$(dirname "$(readlink -f "${THIS}")")"
  while [[ "$path" != "" && ! -e "$path/$1" ]]; do
    path=${path%/*}
  done
  APPDIR="$path"
fi

export PATH="${APPDIR}/usr/bin/main:${PATH}"
export LD_LIBRARY_PATH="${APPDIR}/usr/bin/main/_internal:${LD_LIBRARY_PATH}"

BIN="$APPDIR/usr/bin/main/main"

if [ $NUMBER_OF_ARGS -eq 0 ] ; then
    exec "$BIN"
else
    exec "$BIN" "${args[@]}"
fi
"""
    apprun_file_path = os.path.join(output_dir, "AppRun")

    print(f"Generating AppRun file {apprun_file_path}...")

    with open(apprun_file_path, "w") as f:
        f.write(content)

    # Dar permisos de ejecución al archivo
    os.chmod(apprun_file_path, 0o755)


def generate_appimage():
    """Generate AppImage file for application"""
    output = os.path.join(dist_dir, "RogControlCenter.AppImage")
    print(f"Generating AppImage file to {output}...")
    appimagetool_path = os.path.join(workspace_dir, "resources", "appimagetool")
    cwd = os.getcwd()
    os.makedirs(dist_dir)
    try:
        os.chmod(appimagetool_path, 0o755)
        os.chdir(output_dir)
        subprocess.run(
            [
                appimagetool_path,
                output_dir,
                output,
            ],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            check=True,
        )
    finally:
        os.chdir(cwd)


def package_python():
    """Package Python application"""
    print("Packaging Python code...")
    subprocess.run(
        [
            "pyinstaller",
            "-D",
            "--distpath",
            os.path.join(output_dir, "usr", "bin"),
            "--add-data",
            os.path.join(workspace_dir, "assets") + ":assets",
            os.path.join(workspace_dir, "main.py"),
        ],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
        check=False,
    )


def prepare_workspace():
    """Prepare workspace for build"""
    print("Preparing workspace...")
    shutil.rmtree(output_dir, ignore_errors=True)
    shutil.rmtree(build_dir, ignore_errors=True)
    shutil.rmtree(dist_dir, ignore_errors=True)


def clear_workspace():
    """Clear workspace"""
    print("Cleaning workspace...")
    shutil.rmtree(output_dir, ignore_errors=True)
    shutil.rmtree(build_dir, ignore_errors=True)


prepare_workspace()
package_python()
copy_icon()
generate_desktop()
generate_apprun()
generate_appimage()
clear_workspace()
