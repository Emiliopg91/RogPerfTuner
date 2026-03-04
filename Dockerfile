
    FROM archlinux:latest

    RUN pacman -Sy --noconfirm sudo         && pacman -Scc --noconfirm
    
    # Crear usuario no-root
    RUN useradd -m -G wheel -s /bin/bash builder         && echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

    # Crear directorio donde se montarán los paquetes (como root)
    RUN mkdir -p /pkg /tmp/pkg && chmod 777 /pkg /tmp/pkg

    # Cambiar a usuario builder
    USER builder
    WORKDIR /home/builder

    # Instalar paru desde AUR
    RUN sudo pacman -Sy --noconfirm base-devel git         && git clone https://aur.archlinux.org/paru.git         && cd paru         && makepkg -si --noconfirm         && cd ..         && rm -rf paru         && paru -S --noconfirm asusctl coreutils hicolor-icon-theme hidapi libusb mangohud-git nlohmann-json power-profiles-daemon python python-pip python-yaml qt6-base qt6-charts qt6-svg qtermwidget qtkeychain-qt6 scx-scheds openssl switcheroo-control upower base-devel clang cmake git ninja npm pkgconf pnpm qtcreator unzip zip         && sudo paru -Scc --noconfirm
    WORKDIR /tmp/pkg

    # Ejecutar paru -U por defecto
    CMD ["bash", "-c", "cp -R /pkg/* . && paru -U --noconfirm --nocheck --noinstall"]
    