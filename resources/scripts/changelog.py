# pylint: disable=invalid-name, missing-function-docstring

import os
import subprocess
import sys
import yaml

if len(sys.argv) != 2:
    sys.exit(1)

ARG = sys.argv[1]

PROJ_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
YAML_PATH = os.path.join(PROJ_PATH, "changelog.yaml")
ORDER = ["version", "features", "improvements", "fixes"]


def reorder(map_entry):
    new = {}
    for key in ORDER:
        if key in map_entry:
            new[key] = map_entry[key]
    return new


with open(YAML_PATH, "r", encoding="utf-8") as f:
    data = yaml.safe_load(f)

modified = False
msg = None
if ARG.startswith("version:"):
    msg = ARG.replace("version:", "").strip()
    version, rel = msg.split("-")
    major, minor, patch = version.split(".")
    patch = str(int(patch) + 1)
    msg = f"{major}.{minor}.{patch}-{rel}"

    data = [{"version": msg}] + data
    modified = True
else:
    options = ["fix", "feat", "improve"]
    prefix = ARG.split(":")

    if prefix in options:
        msg = ARG[len(prefix) + 1 :]
        entry = data[0]
        if entry.get(prefix, None) is None:
            entry[prefix] = []
        if msg not in entry[prefix]:
            entry[prefix].append(msg)
            modified = True
    else:
        sys.exit(0)

if modified:
    data = [reorder(e) for e in data]

    with open(YAML_PATH, "w", encoding="utf-8") as f:
        yaml.dump(data, f, sort_keys=False)

    subprocess.run(["git", "add", os.path.basename(YAML_PATH)], check=False)
    subprocess.run(["git", "commit", "-m", f"changelog: {msg}"], check=False)
