#[=======================================================================[.rst:
FindDUMB
-------

Finds the DUMB library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``DUMB::DUMB``
  The DUMB library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``DUMB_FOUND``
  True if the system has the DUMB library.
``DUMB_INCLUDE_DIRS``
  Include directories needed to use DUMB.
``DUMB_LIBRARIES``
  Libraries needed to link to DUMB.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``DUMB_INCLUDE_DIR``
  The directory containing ``dumb.h``.
``DUMB_LIBRARY``
  The path to the DUMB library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_DUMB QUIET dumb)

find_path(
  DUMB_INCLUDE_DIR
  NAMES dumb.h
  HINTS ${PC_DUMB_INCLUDEDIR})

find_library(
  DUMB_LIBRARY_RELEASE
  NAMES dumb
  HINTS ${PC_DUMB_LIBDIR})

find_library(
  DUMB_LIBRARY_DEBUG
  NAMES dumbd
  HINTS ${PC_DUMB_LIBDIR})

include(SelectLibraryConfigurations)
select_library_configurations(DUMB)

if(PC_DUMB_FOUND)
  get_flags_from_pkg_config("${DUMB_LIBRARY}" "PC_DUMB" "_dumb")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DUMB REQUIRED_VARS "DUMB_LIBRARY"
                                                     "DUMB_INCLUDE_DIR")

if(DUMB_FOUND)
  if(NOT TARGET DUMB::DUMB)
    add_library(DUMB::DUMB UNKNOWN IMPORTED)
    set_target_properties(
      DUMB::DUMB
      PROPERTIES IMPORTED_LOCATION "${DUMB_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${DUMB_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_dumb_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_dumb_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_dumb_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_dumb_link_options}")
  endif()

  if(DUMB_LIBRARY_RELEASE)
    set_property(
      TARGET DUMB::DUMB
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties(DUMB::DUMB PROPERTIES IMPORTED_LOCATION_RELEASE
                                                "${DUMB_LIBRARY_RELEASE}")
  endif()
  if(DUMB_LIBRARY_DEBUG)
    set_property(
      TARGET DUMB::DUMB
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(DUMB::DUMB PROPERTIES IMPORTED_LOCATION_DEBUG
                                                "${DUMB_LIBRARY_DEBUG}")
  endif()

  set(DUMB_LIBRARIES DUMB::DUMB)
  set(DUMB_INCLUDE_DIRS DUMB::DUMB)
endif()

mark_as_advanced(DUMB_INCLUDE_DIR DUMB_LIBRARY)
