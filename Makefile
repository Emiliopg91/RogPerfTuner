.PHONY: assets run build lint test clean release increase_version install update

TASKS := python3 resources/scripts/tasks.py

assets:
	@$(TASKS) assets

node_modules: package.json pnpm-lock.yaml
	@pnpm install --frozen-lockfile
	@touch node_modules

run: assets node_modules
	@DEV_MODE=1 pnpm tauri dev

build: assets node_modules
	@pnpm tauri build --no-bundle

lint: node_modules
	@echo "Linting frontend"
	@pnpm lint
	@echo "Linting backend"
	@cd src-tauri && cargo clippy --all-targets

test:
	@cd src-tauri && cargo test --lib

clean:
	@$(TASKS) clean

release:
	@rm -Rf dist && mkdir -p dist && chmod 777 dist
	@cp resources/PKGBUILD dist/PKGBUILD
	@if [ -n "$$GIT_RELEASE" ]; then cp resources/rog-perf-tuner.sh dist/rog-perf-tuner-git.install; else cp resources/rog-perf-tuner.sh dist/rog-perf-tuner.install; fi
	@python3 resources/scripts/release.py

increase_version:
	@$(TASKS) increase_version

install:
	@$(TASKS) install

update:
	@git submodule update --remote submodules/RccDeckyCompanion
