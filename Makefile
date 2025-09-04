.PHONY: config build build_release build_debug run clean release

NUM_CORES := $(shell nproc)
BUILD_TYPE ?= Release
MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
PATCH_DIR := patches
SUBMODULE_DIR := submodules

clean:
	@echo "#######################################################################"
	@echo "######################### Cleaning workspace ##########################"
	@echo "#######################################################################"
	@rm -rf build dist .Debug .Release .qt CMakeCache.txt **/cmake_install.cmake CMakeFiles patches/*.diff.applied assets/scripts assets/bin assets/OpenRGB assets/RccDeckyCompanion **/CMakeFiles
	@cd submodules/httplib && git reset --hard > /dev/null
	@cd submodules/OpenRGB-cppSDK && git reset --hard > /dev/null
	@cd submodules/OpenRGB && git reset --hard > /dev/null && rm -Rf build
	@cd submodules/RccDeckyCompanion && git reset --hard > /dev/null && rm -Rf dist logs out 
	@grep -E 'project\(.*VERSION' CMakeLists.txt | sed -E 's/.*VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/' | xargs echo -n > assets/version

config:
	@echo "#######################################################################"
	@echo "######################## Configuring compiler ########################"
	@echo "#######################################################################"

	@if [ ! -f "patches/OpenRGB-cppSDK.diff.applied" ]; then \
		cd submodules/OpenRGB-cppSDK && git apply ../../patches/OpenRGB-cppSDK.diff && touch ../../patches/OpenRGB-cppSDK.diff.applied; \
	fi

	@if [ ! -f "patches/httplib.diff.applied" ]; then \
		cd submodules/httplib && git apply ../../patches/httplib.diff && touch ../../patches/httplib.diff.applied; \
	fi

	@cmake -B build -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

	@touch .$(BUILD_TYPE)

build: config build_openrgb build_rccdc 
	@echo "#######################################################################"
	@echo "##################### Compiling RogControlCenter ######################"
	@echo "#######################################################################"

	@rm -rf assets/bin RccCore/include/clients/tcp/open_rgb/compatible_devices.hpp RccCore/src/translator/translations.cpp

	@python3 resources/preload/compatible_devices.py
	@python3 resources/preload/translations.py

	@echo "Formatting code..."
	@clang-format -i $$(find RccCommons -name '*.cpp' -o -name '*.hpp')
	@clang-format -i $$(find RccCore -name '*.cpp' -o -name '*.hpp')
	@clang-format -i $$(find RccScripts -name '*.cpp' -o -name '*.hpp')

	@cmake --build build -- -j$(NUM_CORES)
	
	@mkdir assets/bin assets/bin/rgb  assets/bin/performance assets/bin/steam
	@cp build/RccScripts/NextEffect assets/bin/rgb/nextEffect
	@cp build/RccScripts/IncBrightness assets/bin/rgb/incBrightness
	@cp build/RccScripts/DecBrightness assets/bin/rgb/decBrightness
	@cp build/RccScripts/NextProfile assets/bin/performance/nextProfile
	@cp build/RccScripts/SteamRunner assets/bin/steam/run
	@cp build/RccScripts/FlatpakWrapper assets/bin/steam/flatpak

build_openrgb: 
	@if [ ! -f "patches/OpenRGB.diff.applied" ]; then \
		cd submodules/OpenRGB && git apply ../../patches/OpenRGB.diff && touch ../../patches/OpenRGB.diff.applied; \
	fi
	@if [ ! -d "assets/OpenRGB" ]; then \
		echo "#######################################################################" && \
		echo "######################### Compiling OpenRGB ###########################" && \
		echo "#######################################################################" && \
		cd submodules/OpenRGB && ./build.sh \
		./OpenRGB.AppImage --appimage-extract > /dev/null && cp -r squashfs-root ../../assets/OpenRGB; \
	fi

build_rccdc:
	@if [ ! -d "assets/RccDeckyCompanion" ]; then \
		echo "#######################################################################" && \
		echo "#################### Compiling RccDeckyCompanion ######################" && \
		echo "#######################################################################" && \
	    cd submodules/RccDeckyCompanion && ./cli/decky.py build && cp -r out/RccDeckyCompanion ../../assets/RccDeckyCompanion; \
	fi

package:
	@echo "#######################################################################"
	@echo "####################### Generating Dist folder ########################"
	@echo "#######################################################################"
	@mkdir dist dist/RogControlCenter
	@cp ./build/RccCore/RogControlCenter dist/RogControlCenter
	@cp -r assets dist/RogControlCenter

	@echo "#######################################################################"
	@echo "######################### Generating Tar GZip #########################"
	@echo "#######################################################################"
	@tar -czvf dist/RogControlCenter.tgz dist/RogControlCenter/* > /dev/null

	@echo "#######################################################################"
	@echo "######################### Generating AppImage #########################"
	@echo "#######################################################################"
	@rm -rf dist/appimage-fs
	@cp -r dist/RogControlCenter dist/appimage-fs
	@cp resources/AppRun dist/appimage-fs/
	@cp resources/RogControlCenter.desktop dist/appimage-fs/RogControlCenter.desktop
	@cp assets/icons/icon.svg dist/appimage-fs/icon.svg
	@chmod 777 -R resources/appimagetool dist/appimage-fs
	@VERSION=$$(cat assets/version); ./resources/appimagetool -n dist/appimage-fs dist/RogControlCenter.AppImage

release:
	@rm -rf dist
	@make build BUILD_TYPE=Release
	@make package

build_debug:
	@make build BUILD_TYPE=Debug

run: build_debug
	@touch /tmp/fake.AppImage
	@echo "Running 'APPIMAGE=/tmp/fake.AppImage RCC_ASSETS_DIR=$(MAKEFILE_DIR)assets ./build/RccCore/RogControlCenter'"
	@echo ""
	@APPIMAGE=/tmp/fake.AppImage RCC_MODE=DEV RCC_ASSETS_DIR=$(MAKEFILE_DIR)assets ./build/RccCore/RogControlCenter

increase_version:
	@awk '{if ($$0 ~ /project\(.*VERSION/) {match($$0, /([0-9]+)\.([0-9]+)\.([0-9]+)/, v); patch = v[3] + 1; sub(/[0-9]+\.[0-9]+\.[0-9]+/, v[1] "." v[2] "." patch);} print}' CMakeLists.txt > CMakeLists.txt.tmp && mv CMakeLists.txt.tmp CMakeLists.txt
