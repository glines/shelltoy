{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile, jansson, fontconfig, expat, doxygen }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "0.0.4";

  src = fetchgit {
    url = "file:///home/auntieneo/code/shelltoy";
    rev = "50b324f1e75f92b723f6f6babc335c7e4a320370";
    sha256 = "0w3c53mwxkl7626iwwdzv4viyk4r1xfrqwycb0mmr4lkq33qw678";
  };

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig check
    jansson fontconfig expat doxygen
    vimNox  /* For the xxd utility */
  ];
}
