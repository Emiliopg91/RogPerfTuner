import os
import re
import sys

init_file_path = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "lib", "__init__.py")
)

if not os.path.exists(init_file_path):
    raise FileNotFoundError(f"File {init_file_path} not found")


def get_version() -> str:
    with open(init_file_path, "r") as file:
        content = file.read()

    match = re.search(r"^__version__\s*=\s*['\"]([^'\"]+)['\"]", content, re.MULTILINE)
    if match:
        return match.group(1)

    raise ValueError("Variable __version__ not found")


def increment_version():
    with open(init_file_path, "r") as file:
        content = file.read()

    # Expresión regular para buscar la versión
    version_pattern = r"^(__version__\s*=\s*['\"])(\d+)\.(\d+)\.(\d+)(['\"])"
    match = re.search(version_pattern, content, re.MULTILINE)

    if not match:
        raise ValueError("Variable __version__ not found")

    prefix, major, minor, patch, suffix = match.groups()
    major, minor, patch = int(major), int(minor), int(patch)
    patch += 1
    new_version = f"{major}.{minor}.{patch}"

    updated_content = (
        content[: match.start()]
        + f"{prefix}{new_version}{suffix}"
        + content[match.end() :]
    )

    with open(init_file_path, "w") as file:
        file.write(updated_content)

    return new_version


if len(sys.argv) > 1 and sys.argv[1] == "increment":
    print(increment_version())
else:
    print(get_version())
