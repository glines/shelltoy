.PHONY: all
all: result

.PHONY: result
result: 
	nix-build -E '(import <nixpkgs> {}).callPackage ./default.nix {}'

.PHONY: clean
clean:
	rm -f ./result
