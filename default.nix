{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile, jansson, fontconfig, expat, doxygen, python27Packages, git, xorg }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "0.0.7";

  src = fetchgit {
    url = "file:///home/auntieneo/code/shelltoy";
    rev = "refs/tags/${version}";
    sha256 = "18xswv9g1d4hzbyighzximg4bc00izb443wmq5vn226ryclgr34l";
    leaveDotGit = true;  /* Needed for version tags */
    deepClone = true;
  };

  buildInputs = [ cmake libtsm SDL2 glew freetype dejavu_fonts pkgconfig check
    jansson fontconfig expat doxygen
    vimNox  /* For the xxd utility */
    python27Packages.sphinx
    git
    xorg.libX11
  ];
}
