.PHONY: config build build_release build_debug run clean release

NUM_CORES := $(shell nproc)
BUILD_TYPE ?= Debug
MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

clean:
	rm -rf build dist .Debug .Release .qt CMakeCache.txt **/cmake_install.cmake CMakeFiles

config:        
	@if [ ! -f .$(BUILD_TYPE) ]; then \
                make clean; \
        fi
	cmake -B build -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	touch .$(BUILD_TYPE)

build: config
	cmake --build build -- -j$(NUM_CORES)
	if [[ ! -d "assets/OpenRGB" ]]; then cd assets && $(MAKEFILE_DIR)submodules/OpenRGB/OpenRGB.AppImage --appimage-extract && mv squashfs-root OpenRGB; fi

release:
	rm -rf dist
	make build BUILD_TYPE=Release
	mkdir dist dist/RogControlCenter
	cp ./build/core/RogControlCenter dist/RogControlCenter
	cp -r assets dist/RogControlCenter
	tar -czvf dist/RogControlCenter.tgz dist/RogControlCenter/*

build_debug:
	make build BUILD_TYPE=Debug

run: build
	RCC_ASSETS_DIR=$(MAKEFILE_DIR)assets ./build/core/RogControlCenter
