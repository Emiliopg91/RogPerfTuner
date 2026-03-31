# pylint: disable=invalid-name, redefined-outer-name, missing-function-docstring, consider-using-enumerate

import os
import re
import shutil
import subprocess
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

CHANGELOG_YAML_FILE = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "changelog.yaml")
)
CHANGELOG_MD_FILE = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "dist", "changelog.md")
)


def generate_changelog(rel_version, rel_release):
    TAG_REGEX = re.compile(r"^\d+\.\d+\.\d+(-\d)?+$")

    def run_git(cmd):
        return subprocess.check_output(cmd, text=True).strip()

    def get_previous_version_tag():
        tags = run_git(["git", "tag", "--sort=-creatordate"]).splitlines()

        matched_tags = [tag for tag in tags if TAG_REGEX.match(tag)]

        if len(matched_tags) > 0:
            return matched_tags[0]
        return None

    def get_commits_since_tag(tag):
        if not tag:
            cmd = ["git", "log", "--pretty=format:%H %s"]
        else:
            cmd = ["git", "log", f"{tag}..HEAD", "--pretty=format:%H----%s"]
        print(f"Obteniendo commits desde {tag}")
        return run_git(cmd).splitlines()

    tag = get_previous_version_tag()
    commits = get_commits_since_tag(tag)
    commits.reverse()

    entries = {"feature": [], "improve": [], "fix": []}

    for c in commits:
        commit_hash, msg = c.split("----")
        msg = msg.replace("[ci skip]", "").strip()

        for typeEntry, typeEntries in entries.items():
            if msg.startswith(f"[{typeEntry}]"):
                typeEntries.append(
                    f'<td><a href="https://github.com/Emiliopg91/RogPerfTuner/commit/{commit_hash}">{commit_hash[0:7]}</a></td><td>{msg.replace(f"[{typeEntry}]", "").strip().capitalize()}</td>'
                )

    lines: list[str] = ["# No changelog available"]

    if (
        len(entries.get("feature")) > 0
        or len(entries.get("improve")) > 0
        or len(entries.get("fix")) > 0
    ):
        lines: list[str] = [
            "# Changes for release",
            "<table>",
            "<tr><th>Category</th><th>Commit</th><th>Message</th></tr>",
        ]

        for category in [
            ("feature", "New Features"),
            ("improve", "Improvements"),
            ("fix", "Fixes"),
        ]:
            entry, title = category
            for i in range(len(entries.get(entry))):
                line = "<tr>"
                if i == 0:
                    line = f'{line}<td rowspan="{len(entries.get(entry))}" style="vertical-align: top;"><b>{title}</b></td>'
                line = f"{line}{entries.get(entry)[i]}</tr>"
                lines.append(line)

        lines.append("</table>")

    if os.path.exists(CHANGELOG_MD_FILE):
        os.unlink(CHANGELOG_MD_FILE)
    with open(CHANGELOG_MD_FILE, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))


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

    with open(PKGBUILD_FILE, "r", encoding="utf8") as f:
        content = f.read()

    other_sufix = "-git"
    sufix = ""
    ref = "tag=$pkgver-$pkgrel"
    env = ""
    if os.environ.get("IN_TEST", None) is not None:
        version = subprocess.check_output(
            "git tag --sort=-creatordate | head -n 1", shell=True, text=True
        ).strip()
    if os.environ.get("GIT_RELEASE", None) is not None:
        commit = subprocess.check_output(
            ["git", "log", "-1", "--format=%h"], text=True
        ).strip()
        print(f"Commit {commit}")
        commit_count = subprocess.check_output(
            ["git", "rev-list", "--count", f"{version}..HEAD"], text=True
        ).strip()
        print(f"Commit count {commit_count}")
        version = version + ".r" + commit_count
        print(f"Version {version}")
        ref = f"commit={commit}"
        env = "GIT_RELEASE=1"
        other_sufix = ""
        sufix = "-git"

    content = content.replace("<env>", env)
    content = content.replace("<git_ref>", ref)
    content = content.replace("pkgver=<version>", f"pkgver={version}")
    content = content.replace("pkgrel=<release>", f"pkgrel={release}")
    content = content.replace("<sufix>", sufix)
    content = content.replace("<other_sufix>", other_sufix)

    if os.path.exists(PKGBUILD_FILE):
        os.unlink(PKGBUILD_FILE)
    with open(PKGBUILD_FILE, "w", encoding="utf8") as f:
        f.write(content)
        f.flush()

    if os.path.exists(SRCINFO_FILE):
        os.unlink(SRCINFO_FILE)

    print("Updating .SRCINFO file...")
    if shutil.which("makepkg") is not None:
        print("  Using current system makepkg...")
        subprocess.run(
            "makepkg --printsrcinfo > .SRCINFO",
            check=True,
            shell=True,
            cwd=f"{os.getcwd()}/dist",
        )
    else:
        print("  Using containerized makepkg...")
        subprocess.run(
            [
                "docker",
                "run",
                "--rm",
                "-v",
                f"{os.getcwd()}/dist:/repo",
                "epulidogil/arch-srcinfo:latest",
            ],
            check=True,
        )

    generate_changelog(version, release)
