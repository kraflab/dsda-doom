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
``DUMB_DLL``
  Path to the DUMB Windows runtime (any config).
``DUMB_LIBRARY``
  Path to the DUMB library (any config).

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``DUMB_INCLUDE_DIR``
  The directory containing ``dumb.h``.
``DUMB_DLL_RELEASE``
  The path to the DUMB Windows runtime (release config).
``DUMB_DLL_DEBUG``
  The path to the DUMB Windows runtime (debug config).
``DUMB_LIBRARY_RELEASE``
  The path to the DUMB library (release config).
``DUMB_LIBRARY_DEBUG``
  The path to the DUMB library (debug config).

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_DUMB QUIET dumb)

find_path(
  DUMB_INCLUDE_DIR
  NAMES dumb.h
  HINTS "${PC_DUMB_INCLUDEDIR}"
)

find_file(
  DUMB_DLL_RELEASE
  NAMES dumb.dll libdumb.dll
  PATH_SUFFIXES bin
  HINTS "${PC_DUMB_PREFIX}"
)

find_file(
  DUMB_DLL_DEBUG
  NAMES dumbd.dll
  PATH_SUFFIXES bin
  HINTS "${PC_DUMB_PREFIX}"
)

include(SelectDllConfigurations)
select_dll_configurations(DUMB)

find_library(
  DUMB_LIBRARY_RELEASE
  NAMES dumb
  HINTS "${PC_DUMB_LIBDIR}"
)

find_library(
  DUMB_LIBRARY_DEBUG
  NAMES dumbd
  HINTS "${PC_DUMB_LIBDIR}"
)

include(SelectLibraryConfigurations)
select_library_configurations(DUMB)

if(DUMB_DLL OR DUMB_LIBRARY MATCHES ".so|.dylib")
  set(_dumb_library_type SHARED)
else()
  set(_dumb_library_type STATIC)
endif()

get_flags_from_pkg_config("${_dumb_library_type}" "PC_DUMB" "_dumb")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  DUMB
  REQUIRED_VARS "DUMB_LIBRARY" "DUMB_INCLUDE_DIR"
)

if(DUMB_FOUND)
  if(NOT TARGET DUMB::DUMB)
    add_library(DUMB::DUMB ${_dumb_library_type} IMPORTED)
    set_target_properties(
      DUMB::DUMB
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${DUMB_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_dumb_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_dumb_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_dumb_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_dumb_link_options}"
    )
    if(DUMB_DLL)
      set_target_properties(
        DUMB::DUMB 
        PROPERTIES IMPORTED_LOCATION "${DUMB_DLL}"
                   IMPORTED_IMPLIB "${DUMB_LIBRARY}"
      )
    else()
      set_target_properties(
        DUMB::DUMB
        PROPERTIES IMPORTED_LOCATION "${DUMB_LIBRARY}"
                   IMPORTED_IMPLIB "${DUMB_LIBRARY}"
      )
    endif()
  endif()

  if(DUMB_LIBRARY_RELEASE)
    set_property(
      TARGET DUMB::DUMB
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE
    )
    if(DUMB_DLL_RELEASE)
      set_target_properties(
        DUMB::DUMB
        PROPERTIES IMPORTED_LOCATION_RELEASE "${DUMB_DLL_RELEASE}"
                   IMPORTED_IMPLIB_RELEASE "${DUMB_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(
        DUMB::DUMB
        PROPERTIES IMPORTED_LOCATION_RELEASE "${DUMB_LIBRARY_RELEASE}"
                   IMPORTED_IMPLIB_RELEASE "${DUMB_LIBRARY_RELEASE}"
      )
    endif()
  endif()
  if(DUMB_LIBRARY_DEBUG)
    set_property(
      TARGET DUMB::DUMB
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG
    )
    if(DUMB_DLL_DEBUG)
      set_target_properties(
        DUMB::DUMB PROPERTIES IMPORTED_LOCATION_DEBUG "${DUMB_DLL_DEBUG}"
                              IMPORTED_IMPLIB_DEBUG "${DUMB_LIBRARY_DEBUG}"
      )
    else()
      set_target_properties(
        DUMB::DUMB 
        PROPERTIES IMPORTED_LOCATION_DEBUG "${DUMB_LIBRARY_DEBUG}"
                   IMPORTED_IMPLIB_DEBUG "${DUMB_LIBRARY_DEBUG}"
      )
    endif()
  endif()

  set(DUMB_LIBRARIES DUMB::DUMB)
  set(DUMB_INCLUDE_DIRS "${DUMB_INCLUDE_DIR}")
endif()

mark_as_advanced(
  DUMB_INCLUDE_DIR
)
