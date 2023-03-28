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
``libzip_LIBRARY``
  The path to the libzip library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBZIP QUIET libzip)

find_path(
  libzip_INCLUDE_DIR
  NAMES zip.h
  HINTS ${PC_LIBZIP_INCLUDEDIR})

find_library(
  libzip_LIBRARY
  NAMES zip
  HINTS ${PC_LIBZIP_LIBDIR})

if(PC_LIBZIP_FOUND)
  get_flags_from_pkg_config("${libzip_LIBRARY}" "PC_LIBZIP" "_libzip")
elseif(libzip_LIBRARY)
  set(_libzip_link_libraries
      ""
      CACHE FILEPATH "Additional libraries to link to libzip.")
  if(NOT _libzip_link_libraries)
    message(
      "pkg-config is unavailable, if libzip is a static library, linking will fail.\n"
      "Set '_libzip_link_libraries' to the list of libraries to link.")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libzip REQUIRED_VARS "libzip_LIBRARY"
                                                       "libzip_INCLUDE_DIR")

if(libzip_FOUND)
  if(NOT TARGET libzip::zip)
    add_library(libzip::zip UNKNOWN IMPORTED)
    set_target_properties(
      libzip::zip
      PROPERTIES IMPORTED_LOCATION "${libzip_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${libzip_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_libzip_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_libzip_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_libzip_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_libzip_link_options}")
  endif()
endif()

mark_as_advanced(libzip_INCLUDE_DIR libzip_LIBRARY)
