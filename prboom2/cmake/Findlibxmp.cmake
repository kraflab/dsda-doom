# Variables defined:
#  libxmp_FOUND
#  libxmp_INCLUDE_DIR
#  libxmp_LIBRARY

find_package(PkgConfig QUIET)
pkg_check_modules(PC_libxmp QUIET libxmp)

find_library(libxmp_LIBRARY
    NAMES xmp
    HINTS "${PC_libxmp_LIBDIR}")

find_path(libxmp_INCLUDE_DIR
    NAMES xmp.h
    HINTS "${PC_libxmp_INCLUDEDIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libxmp
    REQUIRED_VARS libxmp_LIBRARY libxmp_INCLUDE_DIR)

if(libxmp_FOUND)
    if(NOT TARGET libxmp::xmp)
        add_library(libxmp::xmp UNKNOWN IMPORTED)
        set_target_properties(libxmp::xmp PROPERTIES
            IMPORTED_LOCATION "${libxmp_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${libxmp_INCLUDE_DIR}")
    endif()
endif()

mark_as_advanced(libxmp_LIBRARY libxmp_INCLUDE_DIR)
