import os
import re
import sys

pyproject_toml = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "pyproject.toml"))

if not os.path.exists(pyproject_toml):
    raise FileNotFoundError(f"File {pyproject_toml} not found")


def get_version() -> str:
    """Get version from application module file"""
    with open(pyproject_toml, "r") as file:
        content = file.read()

    match = re.search(r"^version\s*=\s*['\"]([^'\"]+)['\"]", content, re.MULTILINE)
    if match:
        return match.group(1)

    raise ValueError("Variable version not found")


def increment_version():
    """Increment path version number"""
    with open(pyproject_toml, "r") as file:
        content = file.read()

    # Expresión regular para buscar la versión
    version_pattern = r"^(version\s*=\s*['\"])(\d+)\.(\d+)\.(\d+)(['\"])"
    match = re.search(version_pattern, content, re.MULTILINE)

    if not match:
        raise ValueError("Variable version not found")

    prefix, major, minor, patch, suffix = match.groups()
    major, minor, patch = int(major), int(minor), int(patch)
    patch += 1
    new_version = f"{major}.{minor}.{patch}"

    updated_content = content[: match.start()] + f"{prefix}{new_version}{suffix}" + content[match.end() :]

    with open(pyproject_toml, "w") as file:
        file.write(updated_content)

    return new_version


if len(sys.argv) > 1 and sys.argv[1] == "increment":
    print(increment_version())
else:
    print(get_version())
