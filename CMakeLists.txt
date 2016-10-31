cmake_minimum_required(VERSION 3.6)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
project(shelltoy)
set(CMAKE_BUILD_TYPE debug)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#find_package(GLM REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(SYSTEM
    ${GLEW_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    )

add_subdirectory("./src")
