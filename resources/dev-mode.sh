#!/bin/bash

HELPERS=("yay" "paru")
AUR_HELPER=""
RPT_FOLDER="$HOME/.RogPerfTuner"
DEV_FOLDER="/tmp/rog-perf-tuner-dev"

echo "Identifying AUR helper..."

for h in "${HELPERS[@]}"; do
    if command -v "$h" >/dev/null 2>&1; then
        AUR_HELPER="$h"
        break
    fi
done
if [ -z "$AUR_HELPER" ]; then
    echo "No supported AUR helper found"
    exit 1
fi
echo "Detected $AUR_HELPER AUR helper"

if ! command -v gdb &> /dev/null; then
    $AUR_HELPER -S gdb
fi

if [ -d "$DEV_FOLDER" ]; then
    sudo rm -Rf "$DEV_FOLDER"
fi

echo "Downloading source code..."
git clone https://github.com/Emiliopg91/RogPerfTuner "$DEV_FOLDER"
cd "$DEV_FOLDER"
git checkout develop
git submodule update --init --recursive

echo "Compiling and running application..."
cd "$DEV_FOLDER"

RCC_LOG_LEVEL=DEBUG make run
EXIT_CODE=$?

echo ""
echo "RogPerfTuner finished with status $EXIT_CODE"
echo ""

if [[ $EXIT_CODE -ne 0 && $EXIT_CODE -ne 130 ]]; then
    echo "Exited with error. Gathering information..."
    cd "$RPT_FOLDER"
    mkdir dump
    if [ -f "config/config.yaml" ]; then
        cp config/config.yaml dump/config.yaml
    fi

    if [ -f "logs/RogPerfTuner.log" ]; then
        cp logs/RogPerfTuner.log dump/RogPerfTuner.log
    fi
    
    echo "bt" | coredumpctl gdb > dump/gdb.txt

    tar czf ~/rog-perf-tuner-dump.tar.gz -C ~/.RogPerfTuner dump

    echo "Dump report packaged in $HOME/rog-perf-tuner-dump.tar.gz"
    echo ""
fi

read -p 'Press Enter to exit...'