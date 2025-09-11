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
	@cd submodules/OpenRGB-cppSDK && git reset --hard > /dev/null && git submodule foreach git reset --hard > /dev/null
	@cd submodules/OpenRGB && git reset --hard > /dev/null && rm -Rf build
	@cd submodules/RccDeckyCompanion && git reset --hard > /dev/null && rm -Rf dist logs out

config:
	@rm -rf build dist .Debug .Release CMakeCache.txt **/cmake_install.cmake CMakeFiles assets/bin **/CMakeFiles

	@echo "#######################################################################"
	@echo "######################## Configuring compiler ########################"
	@echo "#######################################################################"

	@if [ ! -f "patches/httplib.diff.applied" ]; then \
		cd submodules/httplib && git apply ../../patches/httplib.diff && touch ../../patches/httplib.diff.applied; \
	fi

	@CXX=clang++ CC=clang cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

	@if [ ! -f "compile_commands.json" ]; then \
		ln -s build/compile_commands.json .; \
	fi

	@touch .$(BUILD_TYPE);\

build:
	@if [ ! -f ".$(BUILD_TYPE)" ]; then \
		make config; \
	fi

	@make build_openrgb
	@make build_rccdc

	@echo "#######################################################################"
	@echo "##################### Compiling RogControlCenter ######################"
	@echo "#######################################################################"

	@rm -rf assets/bin

	@python3 resources/preload/constants.py
	@python3 resources/preload/compatible_devices.py
	@python3 resources/preload/translations.py

	@echo "Formatting code..."
	@clang-format -i $$(find RogControlCenter -name '*.cpp' -o -name '*.hpp')

	@if [ ! -f "patches/OpenRGB-cppSDK.diff.applied" ]; then \
		cd submodules/OpenRGB-cppSDK && git apply ../../patches/OpenRGB-cppSDK.diff && touch ../../patches/OpenRGB-cppSDK.diff.applied; \
	fi

	@cmake --build build -- -j$(NUM_CORES)

	@mkdir assets/bin assets/bin/rgb  assets/bin/performance assets/bin/steam
	@cp build/RogControlCenter/NextEffect assets/bin/rgb/nextEffect
	@cp build/RogControlCenter/IncBrightness assets/bin/rgb/incBrightness
	@cp build/RogControlCenter/DecBrightness assets/bin/rgb/decBrightness
	@cp build/RogControlCenter/NextProfile assets/bin/performance/nextProfile
	@cp build/RogControlCenter/SteamRunner assets/bin/steam/run
	@cp build/RogControlCenter/FlatpakWrapper assets/bin/steam/flatpak

build_openrgb:
	@if [ ! -f "patches/OpenRGB.diff.applied" ]; then \
		cd submodules/OpenRGB && git apply ../../patches/OpenRGB.diff && touch ../../patches/OpenRGB.diff.applied; \
	fi
	@if [ ! -d "assets/OpenRGB" ]; then \
		echo "#######################################################################" && \
		echo "######################### Compiling OpenRGB ###########################" && \
		echo "#######################################################################" && \
		cd submodules/OpenRGB && ./build.sh \
		./OpenRGB.AppImage --appimage-extract && cp -r squashfs-root ../../assets/OpenRGB; \
	fi

build_rccdc:
	@if [ ! -d "assets/RccDeckyCompanion" ]; then \
		echo "#######################################################################" && \
		echo "#################### Compiling RccDeckyCompanion ######################" && \
		echo "#######################################################################" && \
	    PYTHONUNBUFFERED=1 cd submodules/RccDeckyCompanion && ./cli/decky.py build && cp -r out/RccDeckyCompanion ../../assets/RccDeckyCompanion; \
	fi

package:
	@echo "#######################################################################"
	@echo "####################### Generating Dist folder ########################"
	@echo "#######################################################################"
	@mkdir dist dist/RogControlCenter
	@cp ./build/RogControlCenter/RogControlCenter dist/RogControlCenter
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
	@VERSION=$$(cat resources/version) ./resources/appimagetool -u "gh-releases-zsync|Emiliopg91|RogControlCenter|latest|RogControlCenter.AppImage.zsync" -n dist/appimage-fs dist/RogControlCenter.AppImage
	@mv RogControlCenter.AppImage.zsync dist

release:
	@rm -rf dist
	@export BUILD_TYPE=Release
	@make build BUILD_TYPE=Release
	@make package

build_debug:
	@make build BUILD_TYPE=Debug

run: build_debug
	@touch /tmp/fake.AppImage
	@echo "Running 'APPIMAGE=/tmp/fake.AppImage RCC_ASSETS_DIR=$(MAKEFILE_DIR)assets ./build/RogControlCenter/RogControlCenter'"
	@echo ""
	@APPIMAGE=/tmp/fake.AppImage RCC_MODE=DEV RCC_ASSETS_DIR=$(MAKEFILE_DIR)assets ./build/RogControlCenter/RogControlCenter

increase_version:
	@awk '{if ($$0 ~ /project\(.*VERSION/) {match($$0, /([0-9]+)\.([0-9]+)\.([0-9]+)/, v); patch = v[3] + 1; sub(/[0-9]+\.[0-9]+\.[0-9]+/, v[1] "." v[2] "." patch);} print}' CMakeLists.txt > CMakeLists.txt.tmp && mv CMakeLists.txt.tmp CMakeLists.txt
