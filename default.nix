{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile, jansson, fontconfig, expat, doxygen, python27Packages }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "git";

  src = fetchgit {
    url = "file:///home/auntieneo/code/shelltoy";
    rev = "8b90674f634b7304ce4f75bd24c9beafa6564479";
    sha256 = "1avy1gp900ld1y882pij3v3bhi0wpxz9fjjivivpmzvikxymhr6w";
  };

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig check
    jansson fontconfig expat doxygen
    vimNox  /* For the xxd utility */
    python27Packages.sphinx
  ];
}
