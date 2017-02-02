cmake_minimum_required(VERSION 3.6)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
project(shelltoy)
enable_testing()

# Get the version string from git
execute_process(
    COMMAND git describe --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE SHELLTOY_QUALIFIED_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
string(REGEX MATCH
    "^[0-9]+\\.[0-9]+\\.[0-9]+"
    SHELLTOY_VERSION
    "${SHELLTOY_QUALIFIED_VERSION}"
    )
string(REGEX REPLACE
    "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1"
    SHELLTOY_MAJOR_VERSION
    "${SHELLTOY_VERSION}"
    )
string(REGEX REPLACE
    "^[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1"
    SHELLTOY_MINOR_VERSION
    "${SHELLTOY_VERSION}"
    )
string(REGEX REPLACE
    "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1"
    SHELLTOY_PATCH_VERSION
    "${SHELLTOY_VERSION}"
    )
message("SHELLTOY_QUALIFIED_VERSION: ${SHELLTOY_QUALIFIED_VERSION}")
message("SHELLTOY_MAJOR_VERSION: ${SHELLTOY_MAJOR_VERSION}")
message("SHELLTOY_MINOR_VERSION: ${SHELLTOY_MINOR_VERSION}")
message("SHELLTOY_PATCH_VERSION: ${SHELLTOY_PATCH_VERSION}")
configure_file(
    "${CMAKE_SOURCE_DIR}/include/shelltoy/version.h.in"
    "${CMAKE_BINARY_DIR}/include/shelltoy/version.h"
    )

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Werror")
endif()

#find_package(GLM REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(libtsm REQUIRED)
find_package(Jansson REQUIRED)
# FIXME: I might need to use the ${FONTCONFIG_DEFINITIONS} variable somewhere
find_package(Fontconfig REQUIRED)

add_subdirectory("./extern")

include_directories(SYSTEM
    ${FONTCONFIG_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${JANSSON_INCLUDE_DIRS}
    ${LIBTSM_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    )

add_subdirectory("./src")

find_package(Check)
if(CHECK_FOUND)
  include_directories(SYSTEM ${CHECK_INCLUDE_DIRS})
  add_subdirectory("./tests")
endif()
