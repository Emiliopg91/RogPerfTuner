.PHONY: config build build_release build_debug run clean release

NUM_CORES := $(shell nproc)
BUILD_TYPE ?= Debug
MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
PATCH_DIR := patches
SUBMODULE_DIR := submodules

clean:
	rm -rf build dist .Debug .Release .qt CMakeCache.txt **/cmake_install.cmake CMakeFiles patches/OpenRGB-cppSDK.diff.applied assets/scripts assets/bin assets/OpenRGB assets/RccDeckyCompanion **/CMakeFiles
	cd submodules/OpenRGB-cppSDK && git reset --hard

config:        
	@if [ ! -f .$(BUILD_TYPE) ]; then \
		make clean; \
	fi
	cmake -B build -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	touch .$(BUILD_TYPE)

build: config apply_patches
	cmake --build build -- -j$(NUM_CORES)

	rm -rf assets/bin
	mkdir assets/bin assets/bin/rgb  assets/bin/performance
	cp build/core/NextEffect assets/bin/rgb/nextEffect
	cp build/core/IncBrightness assets/bin/rgb/incBrightness
	cp build/core/DecBrightness assets/bin/rgb/decBrightness
	cp build/core/NextProfile assets/bin/performance/nextProfile

	if [ ! -d "assets/OpenRGB" ]; then \
	    cd submodules/OpenRGB && ./build.sh && ./OpenRGB.AppImage --appimage-extract && cp -r squashfs-root ../../assets/OpenRGB; \
	fi

	if [ ! -d "assets/RccDeckyCompanion" ]; then \
	    cd submodules/RccDeckyCompanion && ./cli/decky.py build && cp -r out/RccDeckyCompanion ../../assets/RccDeckyCompanion; \
	fi

release:
	rm -rf dist
	make build BUILD_TYPE=Release

	mkdir dist dist/RogControlCenter
	cp ./build/core/RogControlCenter dist/RogControlCenter
	cp -r assets dist/RogControlCenter

	tar -czvf dist/RogControlCenter.tgz dist/RogControlCenter/*

	rm -rf dist/appimage-fs
	cp -r dist/RogControlCenter dist/appimage-fs
	cp resources/AppRun dist/appimage-fs/
	cp resources/RogControlCenter.desktop dist/appimage-fs/RogControlCenter.desktop
	cp assets/icons/rog-logo.svg dist/appimage-fs/icon.svg
	chmod 777 -R resources/appimagetool dist/appimage-fs
	cd dist && ../resources/appimagetool appimage-fs RogControlCenter.AppImage 
	rm -R dist/appimage-fs

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