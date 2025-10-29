#!/bin/bash

UDEV_RULES="/usr/lib/udev/rules.d/60-rog-perf-tuner.rules"
ASSETS_UDEV_RULES="/usr/share/rog-perf-tuner/OpenRGB/60-openrgb.rules"

reload_udev_rules() {
    echo "  🔄 Reloading udev rules..."
    udevadm control --reload-rules
    udevadm trigger
}

stop_application() {
    echo "  ⏹️ Stopping application if running..."
    rog-perf-tuner -k
    return $?
}

relaunch_application() {
    stop_application
    local status=$?
    if [[ $status -eq 0 ]]; then
        echo "  🚀 Relaunching application..."
        nohup bash -c "sleep 2 && /usr/bin/rog-perf-tuner >/dev/null 2>&1 &" >/dev/null 2>&1 &
    fi
}

post_install() {
    echo "📦 Performing install actions..."
    reload_udev_rules
    relaunch_application
}

post_upgrade() {
    echo "🔼 Performing update actions..."
    reload_udev_rules
    relaunch_application
}

post_remove() {
    echo "🗑️ Performing remove actions..."
    reload_udev_rules
    stop_application
}
