{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "0.0.2";

  src = fetchgit {
    url = "file:///home/auntieneo/code/shelltoy";
    rev = "f87a70404dccefbe94ba2a72dc2860c6f86f9df2";
    sha256 = "1w3jlpq12v3vqha2xg9mrjfgx372k2bfna5c1ih6rfsndr83fpia";
  };

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig check
    vimNox  /* For the xxd utility */
  ];
}
