from datetime import datetime
import os
import shutil

import subprocess
import tomllib

workspace_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
py_spec_file = os.path.join(workspace_dir, "main.spec")
pyproject_toml = os.path.join(workspace_dir, "pyproject.toml")
rpm_dir = os.path.join(workspace_dir, "rpm")
rpm_spec_file = os.path.join(rpm_dir, "rcc.spec")
desktop_file = os.path.join(rpm_dir, "RogControlCenter.desktop")
dist_dir = os.path.join(workspace_dir, "dist")
build_dir = os.path.join(workspace_dir, "build")

with open(pyproject_toml, "rb") as f:
    py_project = tomllib.load(f)


def prepare_workspace():
    """Prepare workspace for build"""
    print("Preparing workspace...")
    shutil.rmtree(rpm_dir, ignore_errors=True)
    shutil.rmtree(dist_dir, ignore_errors=True)
    shutil.rmtree(build_dir, ignore_errors=True)
    for d in ["BUILD", "RPMS", "SOURCES", "SPECS", "SRPMS"]:
        os.makedirs(os.path.join(rpm_dir, "rpmbuild", d), exist_ok=True)


def clear_workspace():
    """Clear workspace"""
    print("Cleaning workspace...")
    shutil.rmtree(build_dir, ignore_errors=True)
    shutil.rmtree(rpm_dir, ignore_errors=True)
    os.unlink(py_spec_file)


def package_python():
    """Package Python application"""
    print("Packaging Python code...")
    shutil.copy2(os.path.join(workspace_dir, "pyproject.toml"), os.path.join(workspace_dir, "assets", "pyproject.toml"))
    command = " ".join(
        [
            "pyinstaller",
            "--distpath",
            dist_dir,
            "--add-data",
            os.path.join(workspace_dir, "assets") + ":assets",
            "--onefile",
            os.path.join(workspace_dir, "main.py"),
        ]
    )
    subprocess.run(command, shell=True, check=False, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)


def generate_spec():
    """Generate RPM spec file"""
    print("Generating RPM spec...")

    with open(rpm_spec_file, "w") as file:
        file.write(
            """Name:           """
            + py_project["project"]["name"]
            + """
Version:        """
            + py_project["project"]["version"]
            + """
Release:        1
Summary:        """
            + py_project["project"]["description"]
            + """

License:        MIT
URL:            https://github.com/Emiliopg91/RogControlCenter

%define __brp_strip_none 1
%define __os_install_post %{nil}

%description
"""
            + py_project["project"]["description"]
            + """

%prep
# 

%build
# 

%install
mkdir -p %{buildroot}%{_bindir}
install -m 755 """
            + dist_dir
            + """/main %{buildroot}%{_bindir}/rogcontrolcenter
mkdir -p %{buildroot}%{_datarootdir}/pixmaps
install -m 755 """
            + workspace_dir
            + """/assets/icons/icon-45x45.png %{buildroot}%{_datarootdir}/pixmaps/rogcontrolcenter.png
mkdir -p %{buildroot}%{_datarootdir}/applications
install -m 755 """
            + desktop_file
            + """ %{buildroot}%{_datarootdir}/applications/rogcontrolcenter.desktop
mkdir -p %{buildroot}%{_sysconfdir}/xdg/autostart
install -m 755 """
            + desktop_file
            + """ %{buildroot}%{_sysconfdir}/xdg/autostart/rogcontrolcenter.desktop

%files
%{_bindir}/rogcontrolcenter
%{_datarootdir}/pixmaps/rogcontrolcenter.png
%{_datarootdir}/applications/rogcontrolcenter.desktop
%{_sysconfdir}/xdg/autostart/rogcontrolcenter.desktop

%changelog
* """
            + datetime.now().strftime("%a %b %d %Y")
            + """ Emiliopg91 <ojosdeserbio@gmail.com> - """
            + py_project["project"]["version"]
            + """-1
"""
        )
        file.flush()


def generate_rpm():
    """Generate RPM package"""
    print("Generating RPM package...")
    subprocess.run(
        f'rpmbuild --define "_topdir {rpm_dir}/rpmbuild" -ba {rpm_spec_file}', cwd=rpm_dir, shell=True, check=True
    )
    os.unlink(os.path.join(dist_dir, "main"))
    shutil.move(
        os.path.join(
            rpm_dir,
            "rpmbuild",
            "RPMS",
            "x86_64",
            py_project["project"]["name"] + "-" + py_project["project"]["version"] + "-1.x86_64.rpm",
        ),
        os.path.join(dist_dir, py_project["project"]["name"] + ".x86_64.rpm"),
    )


def generate_desktop():
    """Generate desktop file"""
    print("Generating desktop file...")
    with open(desktop_file, "w") as file:
        file.write(
            """[Desktop Entry]
Name=RogControlCenter
Comment=An utility to manage Asus Rog laptop performance
Exec=/usr/bin/rogcontrolcenter
Icon=/usr/share/pixmaps/rogcontrolcenter.png
Terminal=false
Type=Application
Categories=Utility;
"""
        )
        file.flush()


prepare_workspace()
package_python()
generate_spec()
generate_desktop()
generate_rpm()
clear_workspace()
