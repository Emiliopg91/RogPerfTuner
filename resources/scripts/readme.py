import os
import re
import subprocess

PROJECT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__),"..",".."))
README_FILE = os.path.join(PROJECT_DIR,"README.md")
PKGBUILD_FILE = os.path.join(PROJECT_DIR,"dist","PKGBUILD")

makepkg_lines = subprocess.run("makepkg --printsrcinfo", cwd=os.path.dirname(PKGBUILD_FILE) ,shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout.strip().splitlines()

dependencies = set([])
for line in makepkg_lines:
    line=line.strip()
    if line.startswith("depends = "):
        dep = line.replace("depends = ", "")
        dependencies.add(dep)

dependencies = sorted(dependencies)
command = f"sudo pacman -S --needed {" ".join(dependencies)}"

with open(README_FILE, "r", encoding="utf-8") as f:
    content = f.read()

content = re.sub(
    r'^sudo\s+pacman\s+-S.*$',      
    command, 
    content,
    flags=re.MULTILINE
)

with open(README_FILE, "w", encoding="utf-8") as f:
    f.write(content)
    f.flush()