#[=======================================================================[.rst:
Findlibzip
-------

Finds the libzip library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``libzip::zip``
  The libzip library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``libzip_FOUND``
  True if the system has the libzip library.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``libzip_INCLUDE_DIR``
  The directory containing ``zip.h``.
``libzip_DLL``
  The path to the libzip Windows runtime.
``libzip_LIBRARY``
  The path to the libzip library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_libzip IMPORTED_TARGET libzip)

if(PC_libzip_FOUND)
  if(NOT TARGET libzip::zip)
    add_library(libzip::zip ALIAS PkgConfig::PC_libzip)
  endif()
  set(libzip_FOUND TRUE)
  set(libzip_VERSION ${PC_libzip_VERSION})
  return()
endif()

find_path(
  libzip_INCLUDE_DIR
  NAMES zip.h
)

find_file(
  libzip_DLL
  NAMES zip.dll libzip.dll
  PATH_SUFFIXES bin
)

find_library(
  libzip_LIBRARY
  NAMES zip
)

if(libzip_DLL OR libzip_LIBRARY MATCHES ".so|.dylib")
  set(_libzip_library_type SHARED)
else()
  set(_libzip_library_type STATIC)
endif()

if(_libzip_library_type STREQUAL "STATIC")
  set(libzip_LINK_LIBRARIES "" CACHE STRING "Additional libraries to link to libzip.")
  set(libzip_LINK_DIRECTORIES "" CACHE PATH "Additional directories to search libraries in for libzip.")
  if(NOT libzip_LINK_LIBRARIES)
    message(WARNING
      "pkg-config is unavailable and libzip seems to be static, link failures are to be expected.\n"
      "Set `libzip_LINK_LIBRARIES` to a list of libraries libzip depends on.\n"
      "Set `libzip_LINK_DIRECTORIES` to a list of directories to search for libraries in."
    )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  libzip
  REQUIRED_VARS "libzip_LIBRARY" "libzip_INCLUDE_DIR"
)

if(libzip_FOUND)
  if(NOT TARGET libzip::zip)
    add_library(libzip::zip ${_libzip_library_type} IMPORTED)
    set_target_properties(
      libzip::zip
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${libzip_INCLUDE_DIR}"
                 INTERFACE_LINK_LIBRARIES "${libzip_LINK_LIBRARIES}"
                 INTERFACE_LINK_DIRECTORIES "${libzip_LINK_DIRECTORIES}"
    )
  endif()

  if(libzip_DLL)
    set_target_properties(
      libzip::zip
      PROPERTIES IMPORTED_LOCATION "${libzip_DLL}"
                 IMPORTED_IMPLIB "${libzip_LIBRARY}"
    )
  else()
    set_target_properties(
      libzip::zip
      PROPERTIES IMPORTED_LOCATION "${libzip_LIBRARY}"
    )
  endif()
endif()

mark_as_advanced(
  libzip_INCLUDE_DIR
  libzip_DLL
  libzip_LIBRARY
)
