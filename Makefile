lint:
	@uv run scripts/lint.py

format:
	@uv run scripts/format.py

install:
	@uv run scripts/install.py

dev: install format lint
	@uv run scripts/watch.py $(ARGS)

build: install format lint 
	@uv run scripts/package.py

release: build
	@if [ -n "$(shell git status --porcelain)" ]; then \
		git add .; \
		git commit -m "Commit pending changes before release" > /dev/null; \
	fi
	@VERSION=$(shell uv run scripts/version.py) && \
	git tag $$VERSION && \
	uv run scripts/version.py increment > /dev/null && \
	git add . > /dev/null && \
	git commit -m "Ready for next development iteration" > /dev/null && \
	echo "Succesful released version $$VERSION, please push changes to repository"

%:
	@:
