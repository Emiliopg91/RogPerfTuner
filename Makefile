install:
	@uv run scripts/install.py

format:
	@uv run scripts/format.py
	
lint:
	@uv run scripts/lint.py

dev: install format lint
	@uv run scripts/watch.py $(ARGS)
	
appimage: install format lint
	@uv run scripts/appimage.py
	
rpm: install format lint
	@uv run scripts/rpm.py

%:
	@:
 