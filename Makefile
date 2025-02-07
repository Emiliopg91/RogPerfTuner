lint:
	@python3 scripts/lint.py

format:
	@python3 scripts/format.py

install:
	@python3 scripts/install.py

dev: install format lint
	@python3 scripts/watch.py $(ARGS)

build: install format lint 
	@python3 scripts/package.py

release: build
	@if [ -n "$(shell git status --porcelain)" ]; then \
		git add .; \
		git commit -m "Commit pending changes before release" > /dev/null; \
	fi
	@VERSION=$(shell python3 scripts/version.py) && \
	git tag $$VERSION && \
	python3 scripts/version.py increment > /dev/null && \
	git add . > /dev/null && \
	git commit -m "Ready for next development iteration" > /dev/null && \
	echo "Succesful released version $$VERSION, please push changes to repository"

%:
	@:
