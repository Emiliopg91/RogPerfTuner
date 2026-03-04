import subprocess
import os

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

    dependencies = " ".join(content)

    dockerfile_content = f"""
    FROM archlinux:latest

    RUN pacman -Sy --noconfirm sudo \
        && pacman -Scc --noconfirm
    
    # Crear usuario no-root
    RUN useradd -m -G wheel -s /bin/bash builder \
        && echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

    # Crear directorio donde se montarán los paquetes (como root)
    RUN mkdir -p /pkg /tmp/pkg && chmod 777 /pkg /tmp/pkg

    # Cambiar a usuario builder
    USER builder
    WORKDIR /home/builder

    # Instalar paru desde AUR
    RUN sudo pacman -Sy --noconfirm base-devel git \
        && git clone https://aur.archlinux.org/paru.git \
        && cd paru \
        && makepkg -si --noconfirm \
        && cd .. \
        && rm -rf paru \
        && paru -S --noconfirm {dependencies} \
        && sudo paru -Scc --noconfirm
    WORKDIR /tmp/pkg

    # Ejecutar paru -U por defecto
    CMD ["bash", "-c", "cp -R /pkg/* . && paru -U --noconfirm --nocheck --noinstall"]
    """

    with open("Dockerfile", "w", encoding="utf-8") as f:
        f.write(dockerfile_content)

    subprocess.run(
        [
            "docker",
            "build",
            "-t",
            "epulidogil/rog-perf-tuner-installer-test:latest",
            ".",
        ],
        check=True,
    )

    subprocess.run(
        [
            "docker",
            "push",
            "epulidogil/rog-perf-tuner-installer-test:latest",
        ],
        check=True,
    )

    os.unlink("Dockerfile")

subprocess.run(
    [
        "docker",
        "run",
        "-v",
        os.path.join(os.getcwd(), "dist:/pkg"),
        "epulidogil/rog-perf-tuner-installer-test:latest",
    ],
    check=True,
)
