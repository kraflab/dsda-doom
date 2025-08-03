#[=======================================================================[.rst:
Findmad
-------

Finds the mad library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``mad::mad``
  The mad library

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``mad_INCLUDE_DIR``
  The directory containing ``mad.h``.
``mad_LIBRARY``
  The path to the mad library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_mad IMPORTED_TARGET mad)

if(PC_mad_FOUND)
  if(NOT TARGET mad::mad)
    add_library(mad::mad ALIAS PkgConfig::PC_mad)
  endif()
  set(mad_FOUND TRUE)
  set(mad_VERSION ${PC_mad_VERSION})
  return()
endif()

find_path(
  mad_INCLUDE_DIR
  NAMES mad.h
)

find_library(
  mad_LIBRARY
  NAMES mad mad-0
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  mad
  REQUIRED_VARS
    mad_LIBRARY
    mad_INCLUDE_DIR
)

if(mad_FOUND AND NOT TARGET mad::mad)
  add_library(mad::mad UNKNOWN IMPORTED)
  set_target_properties(
    mad::mad
    PROPERTIES
      IMPORTED_LOCATION "${mad_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${mad_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  mad_INCLUDE_DIR
  mad_LIBRARY
)
