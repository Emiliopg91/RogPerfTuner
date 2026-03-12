SHELL := /bin/bash

.PHONY: clean config format build build_openrgb build_rccdc pkgbuild release build_debug run increase_version test install 

MAKEFLAGS += --no-print-directory
NUM_CORES := $(shell nproc)
BUILD_TYPE ?= Release
MAKEFILE_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
PATCH_DIR := patches
SUBMODULE_DIR := submodules
FROZEN_SUBMODULES := submodules/OpenRGB-cppSDK submodules/OpenRGB 

clean:
	@echo "#######################################################################"
	@echo "######################### Cleaning workspace ##########################"
	@echo "#######################################################################"
	@rm -rf build dist .Debug .Release .qt CMakeCache.txt **/cmake_install.cmake CMakeFiles submodules/patches/*.diff.applied assets/scripts assets/bin assets/OpenRGB assets/RccDeckyCompanion **/CMakeFiles
	@cd submodules/OpenRGB-cppSDK && git reset --hard > /dev/null && git submodule foreach git reset --hard > /dev/null
	@cd submodules/OpenRGB && git reset --hard > /dev/null && git clean -fdx > /dev/null && rm -f CMakeLists.txt
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
	
	@CMAKE_COLOR=1 cmake -B build -G Ninja \
		-DCMAKE_CXX_COMPILER=clang++ \
		-S . \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON 

	@if [ ! -f "compile_commands.json" ]; then \
		ln -s build/compile_commands.json .; \
	fi

	@mkdir build/assets
	@cp -R resources/icons build/assets

	@touch .$(BUILD_TYPE);\

format:
	@clang-format -i $$(find RogPerfTuner -name '*.cpp' -o -name '*.hpp')
	@clang-format -i $$(find Framework -name '*.cpp' -o -name '*.hpp')

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

	@make format

	@cmake --build build -- -j$(NUM_CORES)

	@mkdir build/assets/bin build/assets/bin/rgb  build/assets/bin/performance build/assets/bin/steam
	@cp resources/suggestions.yaml build/assets/suggestions.yaml
	@cp resources/translations.yaml build/assets/translations.yaml
	@cp resources/dev-mode.sh build/assets/dev-mode.sh
	@chmod +x build/assets/dev-mode.sh

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
		cp build/OpenRGB ../../build/assets/OpenRGB/openrgb && \
		cp 60-openrgb.rules ../../build/assets/OpenRGB/60-openrgb.rules; \
	fi

build_rccdc:
	@if [[ -f ".Debug" ]] || command -v steam >/dev/null 2>&1; then \
	    if [ ! -d "build/assets/RccDeckyCompanion" ]; then \
	        echo "#######################################################################"; \
	        echo "#################### Compiling RccDeckyCompanion ######################"; \
	        echo "#######################################################################"; \
	        cd submodules/RccDeckyCompanion && ./cli/decky.py build; \
	        mkdir -p ../../build/assets; \
	        cp -r out/RccDeckyCompanion ../../build/assets/RccDeckyCompanion; \
	    fi; \
	fi

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
	@if [[ -n "$$GIT_RELEASE" ]]; then \
	    cp resources/rog-perf-tuner.sh dist/rog-perf-tuner-git.install; \
	else \
	    cp resources/rog-perf-tuner.sh dist/rog-perf-tuner.install; \
	fi
	@python resources/scripts/release.py

build_debug:
	@DEV_MODE=1 make build BUILD_TYPE=Debug

run: build_debug
	@touch -t 201510220000 build/RogPerfTuner/RogPerfTuner
	@echo "Running 'RCC_ASSETS_DIR=$(MAKEFILE_DIR)build/assets ./build/RogPerfTuner/RogPerfTuner'"
	@echo ""
	@RCC_ASSETS_DIR=$(MAKEFILE_DIR)build/assets ./build/RogPerfTuner/RogPerfTuner

increase_version:
	@awk '{if ($$0 ~ /project\(.*VERSION/) {match($$0, /([0-9]+)\.([0-9]+)\.([0-9]+)/, v); patch = v[3] + 1; sub(/[0-9]+\.[0-9]+\.[0-9]+/, v[1] "." v[2] "." patch);} print}' CMakeLists.txt > CMakeLists.txt.tmp && mv CMakeLists.txt.tmp CMakeLists.txt

test: 
	@GIT_RELEASE=1 make release
	@python3 ./resources/scripts/test.py

install: clean
	@export GIT_RELEASE=1 && make release
	@printf '\nprepare() {\n' >> dist/PKGBUILD
	@printf '    rm -Rf "$$srcdir/RogPerfTuner/*"\n' >> dist/PKGBUILD
	@printf "    rsync -a --exclude='dist' $(MAKEFILE_DIR) \"\$$srcdir/RogPerfTuner/\"\n" >> dist/PKGBUILD
	@printf '    cur=$$(pwd)\n' >> dist/PKGBUILD
	@printf '    cd "$$srcdir/RogPerfTuner/"\n' >> dist/PKGBUILD
	@printf '    rm -Rf "$$srcdir/RogPerfTuner/submodules/RccDeckyCompanion/node_modules"\n' >> dist/PKGBUILD
	@printf '    make clean\n' >> dist/PKGBUILD
	@printf '    cd $$cur\n' >> dist/PKGBUILD
	@printf '}\n' >> dist/PKGBUILD
	@cd dist && makepkg -si

update_submodules: clean
	@echo "#######################################################################"
	@echo "######################## Updating submodules ##########################"
	@echo "#######################################################################"
	@for path in $$(git config --file .gitmodules --get-regexp path | awk '{print $$2}'); do \
		skip=0; \
		for ex in $(FROZEN_SUBMODULES); do \
			if [ "$$path" = "$$ex" ]; then skip=1; fi; \
		done; \
		if [ $$skip -eq 0 ]; then \
			echo "Updating $$path"; \
			branch=$$(git config --file .gitmodules --get submodule.$$path.branch); \
			[ -z "$$branch" ] && branch="main"; \
			cd $$path && git checkout $$branch &>/dev/null && git pull >/dev/null && cd - >/dev/null; \
		fi; \
	done  