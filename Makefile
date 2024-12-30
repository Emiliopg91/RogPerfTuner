.PHONY: dev build

install:
	python3 scripts/install.py

dev: install
	python3 scripts/watch.py

build: install
	python3 scripts/package.py

release:
	python3 scripts/version.py increment