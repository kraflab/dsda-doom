# Variables defined:
#  libxmp_FOUND
#  libxmp_INCLUDE_DIR
#  libxmp_LIBRARY

find_package(PkgConfig QUIET)
pkg_check_modules(PC_libxmp IMPORTED_TARGET libxmp)

if(PC_libxmp_FOUND)
  if(NOT TARGET libxmp::xmp)
    add_library(libxmp::xmp ALIAS PkgConfig::PC_libxmp)
  endif()
  set(libxmp_FOUND TRUE)
  set(libxmp_VERSION ${PC_libxmp_VERSION})
  return()
endif()

find_library(libxmp_LIBRARY
    NAMES xmp
)

find_path(libxmp_INCLUDE_DIR
    NAMES xmp.h
)

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
