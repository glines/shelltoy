{ stdenv, fetchgit, cmake, libtsm, SDL2, glew, freetype, dejavu_fonts, pkgconfig, check, vimNox, gprof2dot, oprofile, jansson, fontconfig, expat, doxygen, python27Packages, git }:

stdenv.mkDerivation rec {
  name = "shelltoy-${version}";
  version = "0.0.5";

  src = fetchgit {
    url = "file:///home/auntieneo/code/shelltoy";
    rev = "refs/tags/${version}";
    sha256 = "13l2jy07ykj0zk6bq8zss8j55ly4wkhffl8p3rddbb3r0bsr63nd";
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
