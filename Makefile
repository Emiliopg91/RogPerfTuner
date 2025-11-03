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
	@cd submodules/OpenRGB-cppSDK && git reset --hard > /dev/null && git submodule foreach git reset --hard > /dev/null
	@cd submodules/OpenRGB && git reset --hard > /dev/null && rm -f CMakeLists.txt
	@cd submodules/RccDeckyCompanion && git reset --hard > /dev/null
	@rm -Rf dist logs out build

config:
	@rm -rf build dist .Debug .Release CMakeCache.txt **/cmake_install.cmake CMakeFiles **/CMakeFiles

	@echo "#######################################################################"
	@echo "######################## Configuring compiler ########################"
	@echo "#######################################################################"

	@if [ ! -f "submodules/patches/OpenRGB-cppSDK.diff.applied" ]; then \
		cd submodules/OpenRGB-cppSDK && git apply ../patches/OpenRGB-cppSDK.diff && touch ../patches/OpenRGB-cppSDK.diff.applied; \
	fi

	@if command -v paru >/dev/null 2>&1; then \
		export AUR_HELPER=paru; \
    elif command -v yay >/dev/null 2>&1; then \
        export AUR_HELPER=yay; \
    fi; \
	CXX=clang++ CC=clang cmake -B build -G Ninja -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $${DEV_MODE:+-DDEV_MODE=1} $${AUR_HELPER:+-DAUR_HELPER=$$AUR_HELPER}

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
	@echo "####################### Compiling RogPerfTuner #######################"
	@echo "#######################################################################"

	@rm -rf build/assets/bin

	@python3 resources/scripts/constants.py

	@echo "Formatting code..."
	@clang-format -i $$(find RogPerfTuner -name '*.cpp' -o -name '*.hpp')

	@cmake --build build -- -j$(NUM_CORES)

	@mkdir build/assets/bin build/assets/bin/rgb  build/assets/bin/performance build/assets/bin/steam
	@cp resources/translations.yaml build/assets/translations.yaml

build_openrgb:
	@if [ ! -f "submodules/patches/OpenRGB.diff.applied" ]; then \
		cd submodules/OpenRGB && git apply ../patches/OpenRGB.diff && touch ../patches/OpenRGB.diff.applied; \
	fi
	@if [ ! -d "build/assets/OpenRGB" ]; then \
		echo "#######################################################################" && \
		echo "######################### Compiling OpenRGB ###########################" && \
		echo "#######################################################################" && \
		cd submodules/OpenRGB && ./build.sh && \
		mkdir -p ../../build/assets/OpenRGB && \
		cp build/openrgb ../../build/assets/OpenRGB/openrgb && \
		cp 60-openrgb.rules ../../build/assets/OpenRGB/60-openrgb.rules; \
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
	@mkdir dist dist/RogPerfTuner
	@cp ./build/RogPerfTuner/RogPerfTuner dist/RogPerfTuner
	@cp -r build/assets dist/RogPerfTuner

	@echo "#######################################################################"
	@echo "######################### Generating AppImage #########################"
	@echo "#######################################################################"
	@rm -rf dist/appimage-fs
	@mkdir -p dist/appimage-fs dist/appimage-fs/usr/bin dist/appimage-fs/usr/share/rog-perf-tuner
	@cp dist/RogPerfTuner/RogPerfTuner dist/appimage-fs/usr/bin/rog-perf-tuner
	@cp -r dist/RogPerfTuner/assets/* dist/appimage-fs/usr/share/rog-perf-tuner
	@cp resources/AppRun dist/appimage-fs/
	@cp resources/RogPerfTuner.desktop dist/appimage-fs/rog-perf-tuner.desktop
	@cp build/assets/icons/icon.svg dist/appimage-fs/icon.svg
	@chmod 777 -R resources/appimagetool dist/appimage-fs
	@rm dist/appimage-fs/usr/share/rog-perf-tuner/OpenRGB/usr/lib/*.so*
	@chmod 777 -R dist
	@ARCH=x86_64 VERSION=$$(cat resources/version) ./resources/appimagetool --comp zstd -u "gh-releases-zsync|Emiliopg91|RogPerfTuner|latest|RogPerfTuner.AppImage.zsync" -n dist/appimage-fs dist/RogPerfTuner.AppImage
	@mv RogPerfTuner.AppImage.zsync dist


pkgbuild:
	@export BUILD_TYPE=Release
	@make build BUILD_TYPE=Release

release:
	@rm -rf dist

	@mkdir dist && chmod 777 -R dist

	@python resources/scripts/constants.py

	@echo "#######################################################################"
	@echo "######################### Generating Release ##########################"
	@echo "#######################################################################"
	@cp resources/PKGBUILD dist/PKGBUILD
	@cp resources/rog-perf-tuner.sh dist/rog-perf-tuner.install
	@python resources/scripts/pkgbuild.py

build_debug:
	@DEV_MODE=1 make build BUILD_TYPE=Debug

run: build_debug
	@touch -t 201510220000 build/RogPerfTuner/RogPerfTuner
	@echo "Running 'APPIMAGE=/tmp/fake.AppImage RCC_ASSETS_DIR=$(MAKEFILE_DIR)build/assets ./build/RogPerfTuner/RogPerfTuner'"
	@echo ""
	@RCC_ASSETS_DIR=$(MAKEFILE_DIR)build/assets ./build/RogPerfTuner/RogPerfTuner

increase_version:
	@awk '{if ($$0 ~ /project\(.*VERSION/) {match($$0, /([0-9]+)\.([0-9]+)\.([0-9]+)/, v); patch = v[3] + 1; sub(/[0-9]+\.[0-9]+\.[0-9]+/, v[1] "." v[2] "." patch);} print}' CMakeLists.txt > CMakeLists.txt.tmp && mv CMakeLists.txt.tmp CMakeLists.txt

install: clean release
	@cd dist && makepkg -si