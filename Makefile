.PHONY: config build build_release build_debug run clean release

NUM_CORES := $(shell nproc)
BUILD_TYPE ?= Debug
MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
PATCH_DIR := patches
SUBMODULE_DIR := submodules

clean:
	rm -rf build dist .Debug .Release .qt CMakeCache.txt **/cmake_install.cmake CMakeFiles patches/OpenRGB-cppSDK.diff.applied
	cd submodules/OpenRGB-cppSDK && git reset --hard

config:        
	@if [ ! -f .$(BUILD_TYPE) ]; then \
		make clean; \
	fi
	cmake -B build -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	touch .$(BUILD_TYPE)

build: config apply_patches
	cmake --build build -- -j$(NUM_CORES)
	if [[ ! -d "assets/OpenRGB" ]]; then cd assets && $(MAKEFILE_DIR)submodules/OpenRGB/OpenRGB.AppImage --appimage-extract && mv squashfs-root OpenRGB; fi

release:
	rm -rf dist
	make build BUILD_TYPE=Release
	mkdir dist dist/RogControlCenter
	cp ./build/core/RogControlCenter dist/RogControlCenter
	cp -r assets dist/RogControlCenter
	tar -czvf dist/RogControlCenter.tgz dist/RogControlCenter/*

apply_patches:
	@if [ ! -f "patches/OpenRGB-cppSDK.diff.applied" ]; then \
		cd submodules/OpenRGB-cppSDK && git apply ../../patches/OpenRGB-cppSDK.diff && touch ../../patches/OpenRGB-cppSDK.diff.applied; \
	fi

build_debug:
	make build BUILD_TYPE=Debug

run: build
	@echo "Running 'RCC_ASSETS_DIR=$(MAKEFILE_DIR)assets ./build/core/RogControlCenter'"
	@echo ""
	@RCC_ASSETS_DIR=$(MAKEFILE_DIR)assets ./build/core/RogControlCenter
