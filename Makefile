.PHONY: all

all:
	@python3 resources/scripts/makefile.py
	
%:
	@python3 resources/scripts/makefile.py $@