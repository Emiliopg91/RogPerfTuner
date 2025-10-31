import os
import re
import shlex
from pathlib import Path

PKGBUILD_FILE = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "dist", "PKGBUILD")
)
PKGBUILD_TEST_FILE = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "dist", "test", "PKGBUILD")
)
SRCINFO_FILE = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "dist", ".SRCINFO")
)
CMAKE_FILE = os.path.abspath(os.path.dirname(__file__) + "/../../CMakeLists.txt")


def parse_pkgbuild(path):
    data = {}
    current_key = None
    array_open = False
    array_values = []

    with open(path, "r", encoding="utf-8") as f:
        for raw_line in f:
            line = raw_line.strip()
            if not line or line.startswith("#"):
                continue

            if array_open:
                if line.startswith(")"):
                    data[current_key] = array_values
                    array_open = False
                    array_values = []
                    current_key = None
                else:
                    matches = re.findall(r"['\"]([^'\"]+)['\"]", line)
                    array_values.extend(matches)
                continue

            m = re.match(r"^([a-zA-Z0-9_]+)=(.*)$", line)
            if not m:
                continue

            key, value = m.groups()
            value = value.strip()

            if value.startswith("(") and not value.endswith(")"):
                array_open = True
                current_key = key
                matches = re.findall(r"['\"]([^'\"]+)['\"]", value)
                array_values.extend(matches)
                continue

            if value.startswith("(") and value.endswith(")"):
                matches = re.findall(r"['\"]([^'\"]+)['\"]", value)
                data[key] = matches
                continue

            if value.startswith('"') or value.startswith("'"):
                value = value[1:-1]
            data[key] = value

    return data


def generate_srcinfo(data, version):
    lines = [f"pkgbase = {data.get('pkgname', 'unknown')}"]

    scalar_keys = ["pkgdesc", "pkgver", "pkgrel", "url", "install"]
    array_keys = [
        "arch",
        "license",
        "makedepends",
        "depends",
        "optdepends",
        "provides",
        "conflicts",
        "source",
        "options",
        "sha256sums",
    ]

    for key in scalar_keys:
        if key in data:
            line = f"\t{key} = {data[key]}"
            for key2 in scalar_keys:
                if key != key2:
                    line = line.replace(f"${key2}", data[key2])
            lines.append(line)

    for key in array_keys:
        if key in data:
            for item in data[key]:
                line = f"\t{key} = {item}"
                for key2 in scalar_keys:
                    if key != key2:
                        line = line.replace(f"${key2}", data[key2])
                lines.append(line)

    # Paquete
    lines.append(f"\npkgname = {data.get('pkgname', 'unknown')}")

    return ("\n".join(lines) + "\n").replace("$pkgver", version)


if __name__ == "__main__":
    content = Path(CMAKE_FILE).read_text(encoding="utf-8")
    match = re.search(
        r"project\s*\(\s*([A-Za-z0-9_]+)\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)",
        content,
        re.IGNORECASE,
    )

    name, version = match.groups()
    version = os.getenv("VERSION", version)
    release = os.getenv("RELEASE", "1")

    with open(PKGBUILD_FILE, "r") as f:
        content = f.read()

    content = content.replace("pkgver=<version>", f"pkgver={version}")
    content = content.replace("pkgrel=<release>", f"pkgrel={release}")

    if os.path.exists(PKGBUILD_FILE):
        os.unlink(PKGBUILD_FILE)
    with open(PKGBUILD_FILE, "w") as f:
        f.write(content)
        f.flush()

    """
    if os.path.exists(PKGBUILD_TEST_FILE):
        os.unlink(PKGBUILD_TEST_FILE)
    with open(PKGBUILD_TEST_FILE, "w") as f:
        f.write(content.replace("#tag=$pkgver",""))
        f.flush()
    """

    if os.path.exists(SRCINFO_FILE):
        os.unlink(SRCINFO_FILE)

    with open(SRCINFO_FILE, "w", encoding="utf-8") as f:
        f.write(generate_srcinfo(parse_pkgbuild(PKGBUILD_FILE), version))
