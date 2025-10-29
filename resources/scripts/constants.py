import os
import json
from pathlib import Path
import re

print("Generating constants preloads")

cmake_file = os.path.abspath(os.path.dirname(__file__) + "/../../CMakeLists.txt")
cmake_time = os.path.getmtime(cmake_file)

plugin_file = os.path.abspath(
    os.path.dirname(__file__) + "/../../submodules/RccDeckyCompanion/package.json"
)
plugin_time = os.path.getmtime(plugin_file)

version_file = os.path.abspath(os.path.dirname(__file__) + "/../../resources/version")

debug_file = os.path.abspath(os.path.dirname(__file__) + "/../../.Debug")
release_file = os.path.abspath(os.path.dirname(__file__) + "/../../.Release")

constants_file = os.path.abspath(
    os.path.dirname(__file__) + "/../../RogPerfTuner/src/utils/constants.cpp"
)
constants_time = os.path.getmtime(constants_file)

content = Path(cmake_file).read_text(encoding="utf-8")
match = re.search(
    r"project\s*\(\s*([A-Za-z0-9_]+)\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)",
    content,
    re.IGNORECASE,
)

name, version = match.groups()
if os.getenv("DEV_MODE", "0") == "1":
    version = version + "-dev"
else:
    version = os.getenv("RCC_VERSION", version + "-1")

with open(plugin_file, "r", encoding="utf-8") as f:
    plugin = json.load(f)

plugin_version = plugin["version"].strip()

if not os.path.exists(version_file) or cmake_time > os.path.getmtime(version_file):
    with open(version_file, "w", encoding="utf-8") as out:
        out.write(version)
    print(f"    Generated {version_file}")

if (
    cmake_time > constants_time
    or plugin_time > constants_time
    or (os.path.exists(debug_file) and os.path.getmtime(debug_file) > constants_time)
    or (
        os.path.exists(release_file) and os.path.getmtime(release_file) > constants_time
    )
):
    with open(constants_file, "r", encoding="utf-8") as f:
        lines = f.readlines()

    version_pattern = re.compile(
        r'(const std::string Constants::APP_VERSION\s*=\s*")[^"]+(";)'
    )
    plugin_pattern = re.compile(
        r'(const std::string Constants::PLUGIN_VERSION\s*=\s*")[^"]+(";)'
    )

    new_lines = []
    for line in lines:
        if version_pattern.search(line):
            line = version_pattern.sub(
                lambda m: f"{m.group(1)}{version}{m.group(2)}", line
            )

        if plugin_pattern.search(line):
            line = plugin_pattern.sub(
                lambda m: f"{m.group(1)}{plugin_version}{m.group(2)}", line
            )

        new_lines.append(line)

    with open(constants_file, "w", encoding="utf-8") as f:
        f.writelines(new_lines)
    print(f"    Updated {constants_file}")
