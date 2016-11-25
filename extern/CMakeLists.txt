include(ExternalProject)

# FIXME: This will never work with Windows or NixOS. It would be nice if jsmn
# had a proper CMakeLists.txt, which would be a very easy patch.
# FIXME: The update check is slow and annoying.
set(JSMN_DIR "${CMAKE_CURRENT_BIN_DIR}/jsmn-prefix/src/jsmn")
set(JSMN_INCLUDE_DIR "${JSMN_DIR}")
set(JSMN_LIBRARY "${JSMN_DIR}/libjsmn.a")
ExternalProject_Add(jsmn
    GIT_REPOSITORY "https://github.com/zserge/jsmn.git"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "make"
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
    )
