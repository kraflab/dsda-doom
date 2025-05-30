#[=======================================================================[.rst:
Findlibopenmpt
-------

Finds the libopenmpt library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``libopenmpt::libopenmpt``
  The libopenmpt library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``libopenmpt_FOUND``
  True if the system has the libopenmpt library.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``libopenmpt_INCLUDE_DIR``
  The directory containing ``libopenmpt/libopenmpt.h``.
``libopenmpt_DLL``
  The path to the libopenmpt Windows runtime.
``libopenmpt_LIBRARY``
  The path to the libopenmpt library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBOPENMPT QUIET libopenmpt)

find_path(
  libopenmpt_INCLUDE_DIR
  NAMES libopenmpt/libopenmpt.h
  HINTS "${PC_LIBOPENMPT_INCLUDEDIR}"
)

find_file(
  libopenmpt_DLL
  NAMES openmpt.dll libopenmpt.dll
  PATH_SUFFIXES bin
  HINTS "${PC_LIBOPENMPT_PREFIX}"
)

find_library(
  libopenmpt_LIBRARY
  NAMES openmpt
  HINTS "${PC_LIBOPENMPT_LIBDIR}"
)

if(libopenmpt_DLL OR libopenmpt_LIBRARY MATCHES ".so|.dylib")
  set(_libopenmpt_library_type SHARED)
else()
  set(_libopenmpt_library_type STATIC)
endif()

get_flags_from_pkg_config("${_libopenmpt_library_type}" "PC_LIBOPENMPT" "_libopenmpt")

if(_libopenmpt_library_type STREQUAL "STATIC" AND NOT PC_LIBOPENMPT_FOUND)
  set(libopenmpt_LINK_LIBRARIES "" CACHE STRING "Additional libraries to link to libopenmpt.")
  set(libopenmpt_LINK_DIRECTORIES "" CACHE PATH "Additional directories to search libraries in for libopenmpt.")
  set(_libopenmpt_link_libraries ${libopenmpt_LINK_LIBRARIES})
  set(_libopenmpt_link_directories ${libopenmpt_LINK_DIRECTORIES})
  if(NOT _libopenmpt_link_libraries)
    message(WARNING
      "pkg-config is unavailable and libopenmpt seems to be static, link failures are to be expected.\n"
      "Set `libopenmpt_LINK_LIBRARIES` to a list of libraries libopenmpt depends on.\n"
      "Set `libopenmpt_LINK_DIRECTORIES` to a list of directories to search for libraries in."
    )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  libopenmpt
  REQUIRED_VARS "libopenmpt_LIBRARY" "libopenmpt_INCLUDE_DIR"
)

if(libopenmpt_FOUND)
  if(NOT TARGET libopenmpt::libopenmpt)
    add_library(libopenmpt::libopenmpt ${_libopenmpt_library_type} IMPORTED)
    set_target_properties(
      libopenmpt::libopenmpt
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${libopenmpt_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_libopenmpt_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_libopenmpt_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_libopenmpt_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_libopenmpt_link_options}"
    )
  endif()

  if(libopenmpt_DLL)
    set_target_properties(
      libopenmpt::libopenmpt
      PROPERTIES IMPORTED_LOCATION "${libopenmpt_DLL}"
                 IMPORTED_IMPLIB "${libopenmpt_LIBRARY}"
    )
  else()
    set_target_properties(
      libopenmpt::libopenmpt
      PROPERTIES IMPORTED_LOCATION "${libopenmpt_LIBRARY}"
    )
  endif()
endif()

mark_as_advanced(
  libopenmpt_INCLUDE_DIR
  libopenmpt_DLL
  libopenmpt_LIBRARY
)
