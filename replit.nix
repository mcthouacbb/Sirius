{ pkgs }: {
	deps = [
		pkgs.python310
		pkgs.clang_12
		pkgs.gcc
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
	];
}