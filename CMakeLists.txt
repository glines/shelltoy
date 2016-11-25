cmake_minimum_required(VERSION 3.6)
#set(CMAKE_DISABLE_SOURCE_CHANGES ON)
#set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
project(shelltoy)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#find_package(GLM REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(libtsm REQUIRED)

add_subdirectory("./extern")

include_directories(SYSTEM
    ${GLEW_INCLUDE_DIR}
    ${JSMN_INCLUDE_DIR}
    ${LIBTSM_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    )

add_subdirectory("./src")

find_package(Check)
if(CHECK_FOUND)
  include_directories(SYSTEM ${CHECK_INCLUDE_DIRS})
  add_subdirectory("./tests")
endif()
