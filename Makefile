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
	@rm -rf build dist .Debug .Release .qt CMakeCache.txt **/cmake_install.cmake CMakeFiles submodules/patches/*.diff.applied assets/scripts assets/bin assets/OpenRGB assets/RccDeckyCompanion **/CMakeFiles
	@cd submodules/json && git reset --hard > /dev/null
	@cd submodules/httplib && git reset --hard > /dev/null
	@cd submodules/OpenRGB-cppSDK && git reset --hard > /dev/null && git submodule foreach git reset --hard > /dev/null
	@cd submodules/OpenRGB && git reset --hard > /dev/null
	@cd submodules/RccDeckyCompanion && git reset --hard > /dev/null
	@rm -Rf dist logs out build

config:
	@rm -rf build dist .Debug .Release CMakeCache.txt **/cmake_install.cmake CMakeFiles **/CMakeFiles

	@echo "#######################################################################"
	@echo "######################## Configuring compiler ########################"
	@echo "#######################################################################"

	@if [ ! -f "submodules/patches/json.diff.applied" ]; then \
		cd submodules/json && git apply ../patches/json.diff && touch ../patches/json.diff.applied; \
	fi
	@if [ ! -f "submodules/patches/httplib.diff.applied" ]; then \
		cd submodules/httplib && git apply ../patches/httplib.diff && touch ../patches/httplib.diff.applied; \
	fi
	@if [ ! -f "submodules/patches/OpenRGB-cppSDK.diff.applied" ]; then \
		cd submodules/OpenRGB-cppSDK && git apply ../patches/OpenRGB-cppSDK.diff && touch ../patches/OpenRGB-cppSDK.diff.applied; \
	fi
	
	@CXX=clang++ CC=clang cmake -B build -G Ninja -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

	@if [ ! -f "compile_commands.json" ]; then \
		ln -s build/compile_commands.json .; \
	fi

	@mkdir build/assets
	@cp -R resources/icons build/assets

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

	@rm -rf build/assets/bin

	@python3 resources/scripts/constants.py

	@echo "Formatting code..."
	@clang-format -i $$(find RogControlCenter -name '*.cpp' -o -name '*.hpp')

	@cmake --build build -- -j$(NUM_CORES)

	@mkdir build/assets/bin build/assets/bin/rgb  build/assets/bin/performance build/assets/bin/steam
	@cp resources/translations.json build/assets/translations.json
	@cp build/RogControlCenter/NextEffect build/assets/bin/rgb/nextEffect
	@cp build/RogControlCenter/IncBrightness build/assets/bin/rgb/incBrightness
	@cp build/RogControlCenter/DecBrightness build/assets/bin/rgb/decBrightness
	@cp build/RogControlCenter/NextProfile build/assets/bin/performance/nextProfile
	@cp build/RogControlCenter/SteamRunner build/assets/bin/steam/run
	@cp build/RogControlCenter/FlatpakWrapper build/assets/bin/steam/flatpak

build_openrgb:
	@if [ ! -f "submodules/patches/OpenRGB.diff.applied" ]; then \
		cd submodules/OpenRGB && git apply ../patches/OpenRGB.diff && touch ../patches/OpenRGB.diff.applied; \
	fi
	@if [ ! -d "build/assets/OpenRGB" ]; then \
		echo "#######################################################################" && \
		echo "######################### Compiling OpenRGB ###########################" && \
		echo "#######################################################################" && \
		cd submodules/OpenRGB && ./build.sh \
		./OpenRGB.AppImage --appimage-extract && mkdir -p ../../build/assets  && cp -r squashfs-root ../../build/assets/OpenRGB && \
		cd ../../build/assets/OpenRGB && \
		mv AppRun OpenRGB.sh && \
		mv AppRun.wrapped OpenRGB && \
		sed -i 's/AppRun.wrapped/OpenRGB/g' OpenRGB.sh && \
		rm usr/lib/libQt5Core.so.5 usr/lib/libQt5Gui.so.5 usr/lib/libQt5Svg.so.5 usr/lib/libQt5Widgets.so.5 usr/lib/libQt5XcbQpa.so.5 usr/lib/libxcb* usr/lib/libjpeg.so.62 usr/lib/libpng16.so.16 usr/bin/OpenRGB.exe; \
	fi

build_rccdc:
	@if [ ! -f "submodules/patches/RccDeckyCompanion.diff.applied" ]; then \
		cd submodules/RccDeckyCompanion && git apply ../patches/RccDeckyCompanion.diff && touch ../patches/RccDeckyCompanion.diff.applied; \
	fi
	@if [ ! -d "build/assets/RccDeckyCompanion" ]; then \
		echo "#######################################################################" && \
		echo "#################### Compiling RccDeckyCompanion ######################" && \
		echo "#######################################################################" && \
	    cd submodules/RccDeckyCompanion && ./cli/decky.py build && mkdir -p ../../build/assets && cp -r out/RccDeckyCompanion ../../build/assets/RccDeckyCompanion; \
	fi

package:
	@echo "#######################################################################"
	@echo "####################### Generating Dist folder ########################"
	@echo "#######################################################################"
	@mkdir dist dist/RogControlCenter
	@cp ./build/RogControlCenter/RogControlCenter dist/RogControlCenter
	@cp -r build/assets dist/RogControlCenter

	@echo "#######################################################################"
	@echo "######################### Generating AppImage #########################"
	@echo "#######################################################################"
	@rm -rf dist/appimage-fs
	@mkdir -p dist/appimage-fs dist/appimage-fs/usr/bin dist/appimage-fs/usr/share/RogControlCenter
	@cp dist/RogControlCenter/RogControlCenter dist/appimage-fs/usr/bin
	@cp -r dist/RogControlCenter/assets/* dist/appimage-fs/usr/share/RogControlCenter
	@cp resources/AppRun dist/appimage-fs/
	@cp resources/RogControlCenter.desktop dist/appimage-fs/RogControlCenter.desktop
	@cp build/assets/icons/icon.svg dist/appimage-fs/icon.svg
	@chmod 777 -R resources/appimagetool dist/appimage-fs
	@ARCH=x86_64 VERSION=$$(cat resources/version) ./resources/appimagetool -u "gh-releases-zsync|Emiliopg91|RogControlCenter|latest|RogControlCenter.AppImage.zsync" -n dist/appimage-fs dist/RogControlCenter.AppImage
	@mv RogControlCenter.AppImage.zsync dist

	@echo "#######################################################################"
	@echo "######################### Generating PKGBUILD #########################"
	@echo "#######################################################################"
ifdef IN_PKGBUILD
	@echo "Skipping PKGBUILD creation"
else
	@cp resources/PKGBUILD dist/PKGBUILD
	@python resources/scripts/pkgbuild.py
endif

release:
	@rm -rf dist
	@export BUILD_TYPE=Release
	@make build BUILD_TYPE=Release
	@make package

build_debug:
	@make build BUILD_TYPE=Debug

run: build_debug
	@touch /tmp/fake.AppImage
	@echo "Running 'APPIMAGE=/tmp/fake.AppImage RCC_ASSETS_DIR=$(MAKEFILE_DIR)build/assets ./build/RogControlCenter/RogControlCenter'"
	@echo ""
	@APPIMAGE=/tmp/fake.AppImage RCC_MODE=DEV RCC_ASSETS_DIR=$(MAKEFILE_DIR)build/assets ./build/RogControlCenter/RogControlCenter

increase_version:
	@awk '{if ($$0 ~ /project\(.*VERSION/) {match($$0, /([0-9]+)\.([0-9]+)\.([0-9]+)/, v); patch = v[3] + 1; sub(/[0-9]+\.[0-9]+\.[0-9]+/, v[1] "." v[2] "." patch);} print}' CMakeLists.txt > CMakeLists.txt.tmp && mv CMakeLists.txt.tmp CMakeLists.txt
