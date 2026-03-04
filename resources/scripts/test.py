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

# Actualizar sistema e instalar dependencias base
RUN pacman -Sy --noconfirm \
    base-devel \
    git \
    sudo \
    && pacman -Scc --noconfirm

# Crear usuario no-root
RUN useradd -m -G wheel -s /bin/bash builder \
    && echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Crear directorio donde se montarán los paquetes (como root)
RUN mkdir -p /pkg && chmod 777 /pkg

# Cambiar a usuario builder
USER builder
WORKDIR /home/builder

# Instalar paru desde AUR
RUN git clone https://aur.archlinux.org/paru.git \
    && cd paru \
    && makepkg -si --noconfirm \
    && cd .. \
    && rm -rf paru

RUN paru -S --noconfirm {dependencies}

WORKDIR /pkg

# Ejecutar paru -U por defecto
CMD ["bash", "-c", "paru -U --noconfirm --nocheck --noinstall && rm -rf /pkg/*"]
"""

with open("Dockerfile", "w", encoding="utf-8") as f:
    f.write(dockerfile_content)

import subprocess

subprocess.run(["docker", "build", "-t", "installer-test", "."], check=True)

import os

os.unlink("Dockerfile")

subprocess.run(
    [
        "docker",
        "run",
        "-it",
        "-v",
        os.path.join(os.getcwd(), "dist:/pkg"),
        "installer-test",
    ],
    check=True,
)
