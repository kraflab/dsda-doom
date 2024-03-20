#[=======================================================================[.rst:
FindFluidSynth
-------

Finds the FluidSynth library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``FluidSynth::libfluidsynth``
  The FluidSynth library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``FluidSynth_FOUND``
  True if the system has the FluidSynth library.
``FluidSynth_INCLUDE_DIRS``
  Include directories needed to use FluidSynth.
``FluidSynth_LIBRARIES``
  Libraries needed to link to FluidSynth.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``FluidSynth_INCLUDE_DIR``
  The directory containing ``FluidSynth.h``.
``FluidSynth_DLL``
  The path to the FluidSynth Windows runtime.
``FluidSynth_LIBRARY``
  The path to the FluidSynth library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_FLUIDSYNTH QUIET fluidsynth)

find_path(
  FluidSynth_INCLUDE_DIR
  NAMES fluidsynth.h
  HINTS "${PC_FLUIDSYNTH_INCLUDEDIR}"
)

find_file(
  FluidSynth_DLL
  NAMES fluidsynth.dll libfluidsynth.dll libfluidsynth-3.dll
  PATH_SUFFIXES bin
  HINTS "${PC_FLUIDSYNTH_PREFIX}"
)

find_library(
  FluidSynth_LIBRARY
  NAMES fluidsynth libfluidsynth
  HINTS "${PC_FLUIDSYNTH_LIBDIR}"
)

if(FluidSynth_DLL OR FluidSynth_LIBRARY MATCHES ".so|.dylib")
  set(_fluidsynth_library_type SHARED)
else()
  set(_fluidsynth_library_type STATIC)
endif()

get_flags_from_pkg_config("${_fluidsynth_library_type}" "PC_FLUIDSYNTH" "_fluidsynth")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  FluidSynth
  REQUIRED_VARS "FluidSynth_LIBRARY" "FluidSynth_INCLUDE_DIR")

if(FluidSynth_FOUND)
  if(NOT TARGET FluidSynth::libfluidsynth)
    add_library(FluidSynth::libfluidsynth ${_fluidsynth_library_type} IMPORTED)
    set_target_properties(
      FluidSynth::libfluidsynth
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FluidSynth_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_fluidsynth_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_fluidsynth_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_fluidsynth_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_fluidsynth_link_options}"
    )
  endif()

  if(FluidSynth_DLL)
    set_target_properties(
      FluidSynth::libfluidsynth
      PROPERTIES IMPORTED_LOCATION "${FluidSynth_DLL}"
                 IMPORTED_IMPLIB "${FluidSynth_LIBRARY}"
    )
  else()
    set_target_properties(
      FluidSynth::libfluidsynth
      PROPERTIES IMPORTED_LOCATION "${FluidSynth_LIBRARY}"
    )
  endif()

  set(FluidSynth_LIBRARIES FluidSynth::libfluidsynth)
  set(FluidSynth_INCLUDE_DIRS "${FluidSynth_INCLUDE_DIR}")
endif()

mark_as_advanced(
  FluidSynth_INCLUDE_DIR
  FluidSynth_DLL
  FluidSynth_LIBRARY
)
