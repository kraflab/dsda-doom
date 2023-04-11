#[=======================================================================[.rst:
FindLibMad
-------

Finds the LibMad library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``LibMad::libmad``
  The LibMad library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``LibMad_FOUND``
  True if the system has the LibMad library.
``LibMad_INCLUDE_DIRS``
  Include directories needed to use LibMad.
``LibMad_LIBRARIES``
  Libraries needed to link to LibMad.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``LibMad_INCLUDE_DIR``
  The directory containing ``LibMad.h``.
``LibMad_DLL``
  The path to the LibMad Windows runtime.
``LibMad_LIBRARY``
  The path to the LibMad library.

#]=======================================================================]

find_path(
  LibMad_INCLUDE_DIR
  NAMES mad.h
)

find_file(
  LibMad_DLL
  NAMES libmad.dll libmad-0.dll
)

find_library(
  LibMad_LIBRARY
  NAMES mad
)

if(LibMad_DLL OR LibMad_LIBRARY MATCHES ".so|.dylib")
  set(_libmad_library_type SHARED)
else()
  set(_libmad_library_type STATIC)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LibMad 
  REQUIRED_VARS "LibMad_LIBRARY" "LibMad_INCLUDE_DIR")

if(LibMad_FOUND)
  if(NOT TARGET LibMad::libmad)
    add_library(LibMad::libmad ${_libmad_library_type} IMPORTED)
    set_target_properties(
      LibMad::libmad
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LibMad_INCLUDE_DIR}"
    )
  endif()

  if(LibMad_DLL)
    set_target_properties(
      LibMad::libmad
      PROPERTIES IMPORTED_LOCATION "${LibMad_DLL}"
                 IMPORTED_IMPLIB "${LibMad_LIBRARY}"
    )
  else()
    set_target_properties(
      LibMad::libmad
      PROPERTIES IMPORTED_LOCATION "${LibMad_LIBRARY}"
    )
  endif()

  set(LibMad_LIBRARIES LibMad::libmad)
  set(LibMad_INCLUDE_DIRS "${LibMad_INCLUDE_DIR}")
endif()

mark_as_advanced(
  LibMad_INCLUDE_DIR
  LibMad_DLL
  LibMad_LIBRARY
)
