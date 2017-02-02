cmake_minimum_required(VERSION 3.6)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
project(shelltoy)
enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add a target to update the Shelltoy version header at build time
configure_file(
    "${CMAKE_SOURCE_DIR}/include/shelltoy/configure_version_header.cmake.in"
    "${CMAKE_BINARY_DIR}/include/shelltoy/configure_version_header.cmake"
    @ONLY
    )
add_custom_target(shelltoy_version ALL
    COMMAND ${CMAKE_COMMAND} -P
      "${CMAKE_BINARY_DIR}/include/shelltoy/configure_version_header.cmake"
    DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/include/shelltoy/configure_version_header.cmake"
      "${CMAKE_CURRENT_SOURCE_DIR}/include/shelltoy/version.h.in"
    )

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

# Use the Shelltoy header files in this project
include_directories(SYSTEM
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    )

# Install the Shelltoy header files
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
    DESTINATION "include"
    FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "${CMAKE_BINARY_DIR}/include/"
    DESTINATION "include"
    FILES_MATCHING PATTERN "*.h")

add_subdirectory("./src")

find_package(Check)
if(CHECK_FOUND)
  include_directories(SYSTEM ${CHECK_INCLUDE_DIRS})
  add_subdirectory("./tests")
endif()
