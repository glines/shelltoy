{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile, jansson, fontconfig, expat, doxygen, python27Packages, git }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "0.0.6";

  src = fetchgit {
    url = "file:///home/auntieneo/code/shelltoy";
    rev = "refs/tags/${version}";
    sha256 = "0ddxbr4900pq08hc9kqr9552ihpn5hrlbhwx4w7fyfrviqznlyg7";
    leaveDotGit = true;  /* Needed for version tags */
    deepClone = true;
  };

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig check
    jansson fontconfig expat doxygen
    vimNox  /* For the xxd utility */
    python27Packages.sphinx
    git
  ];
}
