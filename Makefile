.PHONY: all
all: result

.PHONY: result
result: 
	nix-build -E '((import <nixpkgs> {}).callPackage ./default.nix {}).all'

.PHONY: debug
debug: result
	@exec cgdb -e ./result/bin/ttoy \
		-s ./result-debug/lib/debug/ttoy \
		-d ./src

.PHONY: clean
clean:
	rm -f ./result*
