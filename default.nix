{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile, jansson, fontconfig, expat, doxygen }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "0.0.5";

  src = fetchgit {
    url = "file:///home/auntieneo/code/shelltoy";
    rev = "02fbdac06f1b5ab3d58a83f14bd20c4700717c53";
    sha256 = "15nv5x53rrc78hhwn9blwjs4s907cxs7h8wvsxmw1gq4anvwwb47";
  };

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig check
    jansson fontconfig expat doxygen
    vimNox  /* For the xxd utility */
  ];
}
