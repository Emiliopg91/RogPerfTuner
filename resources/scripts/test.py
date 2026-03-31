import subprocess
import os
import sys

try:
    if os.getenv("GITHUB_ACTIONS", "false").lower() == "false":
        with open("dist/PKGBUILD", "r", encoding="utf-8") as f:
            pkgbuild_content = f.read()

        pkgbuild_content = pkgbuild_content.split("install=rog-perf-tuner")[0]
        pkgbuild_content = (
            (
                pkgbuild_content.split("depends=(")[1]
                + "\n"
                + pkgbuild_content.split("depends=(")[2]
            )
            .strip()
            .replace("  ", "")
        ).splitlines()

        content = []
        for line in pkgbuild_content:
            if (
                line == ")"
                or line.strip() == "make"
                or line.strip() == ")"
                or len(line.strip()) == 0
            ):
                continue
            content.append(line.replace("'", ""))

        dependencies = " ".join(content[:-1])

        dockerfile_content = f"""FROM archlinux@sha256:237637c52069930ed7fc76c76f04b6b6b9cf82c5222ae002b7932df983cb69c1 AS base

RUN pacman -Sy --noconfirm sudo base-devel git \\
    && pacman -Scc --noconfirm 

RUN useradd -m -G wheel -s /bin/bash builder \\
    && echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

RUN mkdir -p /pkg /tmp/pkg && chmod 777 /pkg /tmp/pkg

USER builder
WORKDIR /pkg

CMD ["bash", "-c", "makepkg --printsrcinfo > .SRCINFO"]

FROM base AS builder

WORKDIR /tmp

RUN git clone https://aur.archlinux.org/paru.git \\
    && cd paru \\
    && makepkg -si --noconfirm \\
    && cd .. \\
    && rm -rf paru

RUN paru -S --noconfirm {dependencies} && sudo paru -Scc --noconfirm

WORKDIR /tmp/pkg

CMD ["bash", "-c", "cp -R /pkg/* . && export IN_TEST=1 && paru -U --noconfirm --nocheck --noinstall && cp $(ls *.pkg.tar.zst) /pkg"]
    """

        with open("Dockerfile", "w", encoding="utf-8") as f:
            f.write(dockerfile_content)

        subprocess.run(
            [
                "docker",
                "build",
                "--target",
                "base",
                "-t",
                "epulidogil/rog-perf-tuner-srcinfo:latest",
                ".",
            ],
            check=True,
        )

        subprocess.run(
            [
                "docker",
                "build",
                "--target",
                "builder",
                "-t",
                "epulidogil/rog-perf-tuner-test-installer:latest",
                ".",
            ],
            check=True,
        )

        subprocess.run(
            [
                "docker",
                "login",
                "docker.io",
            ],
            check=True,
        )

        subprocess.run(
            [
                "docker",
                "push",
                "epulidogil/rog-perf-tuner-srcinfo:latest",
            ],
            check=True,
        )

        subprocess.run(
            [
                "docker",
                "push",
                "epulidogil/rog-perf-tuner-test-installer:latest",
            ],
            check=True,
        )

        os.unlink("Dockerfile")

    subprocess.run(
        [
            "docker",
            "run",
            "--rm",
            "--name",
            "rog-perf-tuner-test-installer",
            "-v",
            os.path.join(os.getcwd(), "dist:/pkg"),
            "epulidogil/rog-perf-tuner-test-installer:latest",
        ],
        check=True,
    )

except subprocess.CalledProcessError as e:
    sys.exit(e.returncode)
