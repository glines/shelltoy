{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile, jansson, fontconfig, expat, doxygen, python27Packages, git, xorg }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "dev";

  src = ./.;

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig check
    jansson fontconfig expat doxygen
    vimNox  /* For the xxd utility */
    python27Packages.sphinx
    git
    xorg.libX11
  ];

  preConfigure = ''
    # NOTE: The default nix hooks for cmake also use the ./build
    # directory...  we just remove this directory in advance so the
    # build is not contaminated.
    rm -rf ./build
  '';
}
