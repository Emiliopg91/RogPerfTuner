# Use a base image with the required development tools
FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# Install essential packages and dependencies
RUN apt-get update && apt-get install -y \
    git \
    build-essential \
    qtcreator \
    qtbase5-dev \
    qtchooser \
    qt5-qmake \
    qtbase5-dev-tools \
    libusb-1.0-0-dev \
    libhidapi-dev \
    pkgconf \
    libmbedtls-dev \
    qttools5-dev-tools \
    && rm -rf /var/lib/apt/lists/*

# Set the default command to clone, build, and copy the binary
CMD ["/bin/bash", "-c", "\
    cd /input/build && \
    qmake ../OpenRGB.pro && \
    make -j$(nproc)"]
