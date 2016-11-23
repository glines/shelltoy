{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "0.0.3";

  src = fetchgit {
    url = "file:///home/auntieneo/code/shelltoy";
    rev = "fe4490a90789444a9ff50da00e96af1f5fdaaf77";
    sha256 = "10wq83iv6m683r2l6xhziiqilsxgkn4g35y2ypvjllrch6k9xw21";
  };

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig check
    vimNox  /* For the xxd utility */
  ];
}
