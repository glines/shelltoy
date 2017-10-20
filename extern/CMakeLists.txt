include(ExternalProject)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/libtsm"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/libtsm")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/libtsm_install")

ExternalProject_Add(tsm_build
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libtsm"
    CONFIGURE_COMMAND sh ./libtsm/autogen.sh
    BUILD_COMMAND make
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND echo
    )

set(TSM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libtsm/src/tsm" PARENT_SCOPE)
set(TSM_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/libtsm/.libs/libtsm.a" PARENT_SCOPE)
