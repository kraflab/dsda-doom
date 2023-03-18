#[=======================================================================[.rst:
FindOgg
-------

Finds the Ogg library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Ogg::ogg``
  The Ogg library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Ogg_FOUND``
  True if the system has the Ogg library.
``Ogg_INCLUDE_DIRS``
  Include directories needed to use Ogg.
``Ogg_LIBRARIES``
  Libraries needed to link to Ogg.

The following variables are also set for compatibility with upstream:
``OGG_INCLUDE_DIR``
``OGG_INCLUDE_DIRS``
``OGG_LIBRARY``
``OGG_LIBRARIES``

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Ogg_INCLUDE_DIR``
  The directory containing ``ogg/ogg.h``.
``Ogg_LIBRARY``
  The path to the Ogg library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_OGG QUIET ogg)

find_path(
  Ogg_INCLUDE_DIR
  NAMES ogg/ogg.h
  HINTS ${PC_OGG_INCLUDEDIR})

find_library(
  Ogg_LIBRARY
  NAMES ogg
  HINTS ${PC_OGG_LIBDIR})

if(PC_OGG_FOUND)
  get_flags_from_pkg_config("${Ogg_LIBRARY}" "PC_OGG" "_ogg")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ogg REQUIRED_VARS "Ogg_LIBRARY"
                                                    "Ogg_INCLUDE_DIR")

if(Ogg_FOUND)
  if(NOT TARGET Ogg::ogg)
    add_library(Ogg::ogg UNKNOWN IMPORTED)
    set_target_properties(
      Ogg::ogg
      PROPERTIES IMPORTED_LOCATION "${Ogg_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${Ogg_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_ogg_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_ogg_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_ogg_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_ogg_link_options}")
  endif()

  set(Ogg_LIBRARIES Ogg::ogg)
  set(Ogg_INCLUDE_DIRS Ogg::ogg)
  set(OGG_LIBRARY Ogg::ogg)
  set(OGG_LIBRARIES Ogg::ogg)
  set(OGG_INCLUDE_DIR Ogg::ogg)
  set(OGG_INCLUDE_DIRS Ogg::ogg)
endif()

mark_as_advanced(Ogg_INCLUDE_DIR Ogg_LIBRARY)
