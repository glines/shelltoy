# Script that looks for libtsm, the Terminal-emulator State Machine library
# Defined by this script:
#   LIBTSM_FOUND - Have libtsm
#   LIBTSM_INCLUDE_DIR - Directory with libtsm header files
#   LIBTSM_LIBRARY - Path to the libtsm library

# Use pkg-config to gather hints as to the library's location
find_package(PkgConfig)
pkg_check_modules(PC_LIBTSM QUIET libtsm)

find_path(LIBTSM_INCLUDE_DIR libtsm.h
    HINTS ${PC_LIBTSM_INCLUDEDIR} ${PC_LIBTSM_INCLUDE_DIRS})
find_library(LIBTSM_LIBRARY NAMES tsm libtsm
    HINTS ${PC_LIBTSM_LIBDIR} ${PC_LIBTSM_LIBRARY_DIRS})

# Handles QUIETLY and REQUIRED arguments and sets LIBTSM_FOUND
find_package_handle_standard_args(libtsm DEFAULT_MSG
    LIBTSM_LIBRARY LIBTSM_INCLUDE_DIR)
