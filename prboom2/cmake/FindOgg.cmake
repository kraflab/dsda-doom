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
``Ogg_DLL``
  The path to the Ogg Windows runtime.
``Ogg_LIBRARY``
  The path to the Ogg library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_ogg IMPORTED_TARGET ogg)

if(PC_ogg_FOUND)
  if(NOT TARGET Ogg::ogg)
    add_library(Ogg::ogg ALIAS PkgConfig::PC_ogg)
  endif()
  set(Ogg_FOUND TRUE)
  set(Ogg_VERSION ${PC_ogg_VERSION})
  return()
endif()

find_path(
  Ogg_INCLUDE_DIR
  NAMES ogg/ogg.h
)

find_file(
  Ogg_DLL
  NAMES ogg.dll libogg.dll libogg-0.dll
  PATH_SUFFIXES bin
)

find_library(
  Ogg_LIBRARY
  NAMES ogg
)

if(Ogg_DLL OR Ogg_LIBRARY MATCHES ".so|.dylib")
  set(_ogg_library_type SHARED)
else()
  set(_ogg_library_type STATIC)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Ogg
  REQUIRED_VARS "Ogg_LIBRARY" "Ogg_INCLUDE_DIR"
)

if(Ogg_FOUND)
  if(NOT TARGET Ogg::ogg)
    add_library(Ogg::ogg ${_ogg_library_type} IMPORTED)
    set_target_properties(
      Ogg::ogg
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Ogg_INCLUDE_DIR}"
    )
  endif()

  if(Ogg_DLL)
    set_target_properties(
      Ogg::ogg
      PROPERTIES IMPORTED_LOCATION "${Ogg_DLL}"
                 IMPORTED_IMPLIB "${Ogg_LIBRARY}"
    )
  else()
    set_target_properties(
      Ogg::ogg
      PROPERTIES IMPORTED_LOCATION "${Ogg_LIBRARY}"
    )
  endif()

  set(Ogg_INCLUDE_DIRS "${Ogg_INCLUDE_DIR}")
  set(OGG_INCLUDE_DIR "${Ogg_INCLUDE_DIR}")
  set(OGG_INCLUDE_DIRS "${Ogg_INCLUDE_DIR}")
  set(Ogg_LIBRARIES Ogg::ogg)
  set(OGG_LIBRARY Ogg::ogg)
  set(OGG_LIBRARIES Ogg::ogg)
endif()

mark_as_advanced(
  Ogg_INCLUDE_DIR
  Ogg_DLL
  Ogg_LIBRARY
)
