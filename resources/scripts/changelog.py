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

if ARG.startswith("version:"):
    data = [{"version": ARG.replace("version:", "").strip()}] + data
elif ARG.startswith("fix:"):
    entry = data[0]
    print(f"{entry}")
    if entry.get("fixes", None) is None:
        entry["fixes"] = []
    entry["fixes"].append(ARG.replace("fix:", "").strip())
elif ARG.startswith("feat:"):
    entry = data[0]
    print(f"{entry}")
    if entry.get("features", None) is None:
        entry["features"] = []
    entry["features"].append(ARG.replace("feat:", "").strip())
elif ARG.startswith("improve:"):
    entry = data[0]
    print(f"{entry}")
    if entry.get("improvements", None) is None:
        entry["improvements"] = []
    entry["improvements"].append(ARG.replace("improve:", "").strip())
else:
    sys.exit(0)

data = [reorder(e) for e in data]

with open(YAML_PATH, "w", encoding="utf-8") as f:
    yaml.dump(data, f, sort_keys=False)


subprocess.run(["git", "add", os.path.basename(YAML_PATH)], check=False)
subprocess.run(["git", "commit", "-m", "docs: Changelog"], check=False)
