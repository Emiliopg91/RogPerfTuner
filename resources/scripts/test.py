import subprocess
import os
import sys

try:
        dockerfile_content = f"""FROM archlinux@sha256:237637c52069930ed7fc76c76f04b6b6b9cf82c5222ae002b7932df983cb69c1 AS base
RUN echo {dependencies}

RUN pacman -Sy --noconfirm sudo base-devel git \\
    && pacman -Scc --noconfirm 

RUN useradd -m -G wheel -s /bin/bash builder \\
    && echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

RUN mkdir -p /pkg /tmp/pkg && chmod 777 /pkg /tmp/pkg

USER builder
WORKDIR /pkg

CMD ["bash", "-c", "makepkg --printsrcinfo > .SRCINFO"]
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
