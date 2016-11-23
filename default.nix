{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "0.0.1";

  src = fetchgit {
    url = "file:///home/auntieneo/code/shelltoy";
    rev = "8c3e5938d91a1251adc8cd4e97b4507bff14f850";
    sha256 = "0mnc0pv9cavcz1lg4cwrhj7hslss47w05z2lhqwp5x5jkgsq9c8c";
  };

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig check
    vimNox  /* For the xxd utility */
    gprof2dot oprofile  /* For profiling */
  ];
}
