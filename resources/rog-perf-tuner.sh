#!/bin/bash

UDEV_RULES="/usr/lib/udev/rules.d/60-rog-perf-tuner.rules"
ASSETS_UDEV_RULES="/usr/share/rog-perf-tuner/OpenRGB/60-openrgb.rules"


post_install() {
    echo "Installing udev rules..."

    mv "$ASSETS_UDEV_RULES" "$UDEV_RULES"
    chmod 644 "$UDEV_RULES"

    udevadm control --reload-rules
    udevadm trigger
}

post_upgrade() {
    echo "Updating udev rules..."

    mv "$ASSETS_UDEV_RULES" "$UDEV_RULES"
    chmod 644 "$UDEV_RULES"

    udevadm control --reload-rules
    udevadm trigger
}

post_remove() {
    echo "Removing udev rules..."

    rm "$UDEV_RULES"

    udevadm control --reload-rules
    udevadm trigger
}