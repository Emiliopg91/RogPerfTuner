lint:
	@python3 scripts/lint.py

format:
	@python3 scripts/format.py

install:
	@python3 scripts/install.py

dev: install format lint
	@python3 scripts/watch.py

build: install format lint 
	@python3 scripts/package.py

release: build
	@python3 scripts/version.py increment