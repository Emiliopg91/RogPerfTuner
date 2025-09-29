import os
import re
from pathlib import Path

PKGBUILD_FILE=os.path.abspath(os.path.join(os.path.dirname(__file__),"..","..","dist","PKGBUILD"))
CMAKE_FILE = os.path.abspath(os.path.dirname(__file__) + "/../../CMakeLists.txt")

content = Path(CMAKE_FILE).read_text(encoding="utf-8")
match = re.search(
    r"project\s*\(\s*([A-Za-z0-9_]+)\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)",
    content,
    re.IGNORECASE,
)
name, version = match.groups()

with open(PKGBUILD_FILE, "r") as f:
    content = f.read()

content=content.replace("pkgver=<version>", f"pkgver={version}")

with open(PKGBUILD_FILE, "w") as f:
    f.write(content)
    f.flush()

