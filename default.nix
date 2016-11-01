{ stdenv, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "0.0.1";

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig ];
}
