#!/bin/bash

echo "Identifying AUR helper..."
helpers=("yay" "paru")
AUR_HELPER=""
for h in "${helpers[@]}"; do
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

if [ ! -d "$HOME/RogPerfTuner-Debug" ]; then
    echo "Downloading source code..."
    git clone https://github.com/Emiliopg91/RogPerfTuner "$HOME/RogPerfTuner-Debug"
    cd $HOME/RogPerfTuner-Debug
    git submodule update --init --recursive
else
    cd $HOME/RogPerfTuner-Debug
    echo "Cleaning and updating project..."
    make clean
    git reset --hard
    git pull
fi

echo "Compiling and running application..."
cd ~/RogPerfTuner-Debug
RCC_LOG_LEVEL=DEBUG make clean run
EXIT_CODE=$?

if [[ $EXIT_CODE -ne 0 && $EXIT_CODE -ne 130 ]]; then
    echo "Exited with error. Gathering information..."
    cd ~/.RogPerfTuner/
    mkdir dump
    if [ -f "config/config.yaml" ]; then
        cp config/config.yaml dump/config.yaml
    fi

    if [ -f "logs/RogPerfTuner.log" ]; then
        cp logs/RogPerfTuner.log dump/RogPerfTuner.log
    fi
    
    pid=$(cat /run/user/$UID/RogPerfTuner.lock | tr -d '\r\n')
    echo "bt" | coredumpctl gdb $pid > dump/gdb.txt


    tar czf dump.tar.gz -C ~/.RogPerfTuner dump
    cp dump.tar.gz ~/rog-perf-tuner-dump.tar.gz

    echo "Dump report packaged in $HOME/rog-perf-tuner-dump.tar.gz"
    read -p 'Press Enter to exit...'
fi
