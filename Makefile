install:
	@uv run scripts/install.py

format:
	@uv run scripts/format.py
	
lint:
	@uv run scripts/lint.py

dev: install format lint
	@uv run scripts/watch.py $(ARGS)

build: install format lint 
	@uv run scripts/package.py

%:
	@:
 