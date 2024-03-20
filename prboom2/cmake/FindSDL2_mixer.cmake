#[=======================================================================[.rst:
FindSDL2_mixer
-------

Finds the SDL2_mixer library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides either of the following imported targets, if found:

``SDL2_mixer::SDL2_mixer``
  The shared SDL2_mixer library
``SDL2_mixer::SDL2_mixer-static``
  The static SDL2_mixer library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SDL2_mixer_FOUND``
  True if the system has the SDL2_mixer library.
``SDL2_mixer_INCLUDE_DIRS``
  Include directories needed to use SDL2_mixer.
``SDL2_mixer_LIBRARIES``
  Libraries needed to link to SDL2_mixer.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``SDL2_mixer_INCLUDE_DIR``
  The directory containing ``SDL2_mixer.h``.
``SDL2_mixer_DLL_RELEASE``
  The path to the SDL2_mixer Windows runtime (release config).
``SDL2_mixer_DLL_DEBUG``
  The path to the SDL2_mixer Windows runtime (debug config).
``SDL2_mixer_LIBRARY_RELEASE``
  The path to the SDL2_mixer library (release config).
``SDL2_mixer_LIBRARY_DEBUG``
  The path to the SDL2_mixer library (debug config).

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_SDL2_MIXER QUIET SDL2_mixer)

find_path(
  SDL2_mixer_INCLUDE_DIR
  NAMES SDL_mixer.h
  PATH_SUFFIXES SDL2
  HINTS "${PC_SDL2_MIXER_INCLUDEDIR}"
)

find_file(
  SDL2_mixer_DLL_RELEASE
  NAMES SDL2_mixer.dll
  PATH_SUFFIXES bin
  HINTS "${PC_SDL2_MIXER_PREFIX}"
)

find_file(
  SDL2_mixer_DLL_DEBUG
  NAMES SDL2_mixerd.dll
  PATH_SUFFIXES bin
  HINTS "${PC_SDL2_MIXER_PREFIX}"
)

include(SelectDllConfigurations)
select_dll_configurations(SDL2_mixer)

find_library(
  SDL2_mixer_LIBRARY_RELEASE
  NAMES SDL2_mixer SDL2_mixer-static
  HINTS "${PC_SDL2_MIXER_LIBDIR}"
)

find_library(
  SDL2_mixer_LIBRARY_DEBUG
  NAMES SDL2_mixerd SDL2_mixer-staticd
  HINTS "${PC_SDL2_MIXER_LIBDIR}"
)

include(SelectLibraryConfigurations)
select_library_configurations(SDL2_mixer)

if(SDL2_mixer_DLL OR SDL2_mixer_LIBRARY MATCHES ".so|.dylib")
  set(_sdl2_mixer_library_type SHARED)
else()
  set(_sdl2_mixer_library_type STATIC)
endif()

get_flags_from_pkg_config("${_sdl2_mixer_library_type}" "PC_SDL2_MIXER" "_sdl2_mixer")

if(_SDL2_mixer_library_type STREQUAL "STATIC" AND NOT PC_SDL2_MIXER_FOUND)
  set(SDL2_mixer_LINK_LIBRARIES "" CACHE STRING "Additional libraries to link to SDL2_mixer.")
  set(SDL2_mixer_LINK_DIRECTORIES "" CACHE PATH "Additional directories to search libraries in for SDL2_mixer.")
  set(_sdl2_mixer_link_libraries ${SDL2_mixer_LINK_LIBRARIES})
  set(_sdl2_mixer_link_directories ${SDL2_mixer_LINK_DIRECTORIES})
  if(NOT _sdl2_mixer_link_libraries)
    message(WARNING
      "pkg-config is unavailable and SDL2_mixer seems to be static, link failures are to be expected.\n"
      "Set `SDL2_mixer_LINK_LIBRARIES` to a list of libraries SDL2_mixer depends on.\n"
      "Set `SDL2_mixer_LINK_DIRECTORIES` to a list of directories to search for libraries in."
    )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SDL2_mixer
  REQUIRED_VARS "SDL2_mixer_LIBRARY" "SDL2_mixer_INCLUDE_DIR")

if(SDL2_mixer_FOUND)
  if(_sdl2_mixer_library_type STREQUAL "SHARED")
    set(_sdl2_mixer_target_name SDL2_mixer::SDL2_mixer)
  else()
    set(_sdl2_mixer_target_name SDL2_mixer::SDL2_mixer-static)
  endif()
  if(NOT TARGET ${_sdl2_mixer_target_name})
    add_library(${_sdl2_mixer_target_name} ${_sdl2_mixer_library_type} IMPORTED)
    set_target_properties(
      ${_sdl2_mixer_target_name}
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SDL2_mixer_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_sdl2_mixer_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_sdl2_mixer_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_sdl2_mixer_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_sdl2_mixer_link_options}"
    )
  endif()

  if(SDL2_mixer_DLL)
    set_target_properties(
      ${_sdl2_mixer_target_name}
      PROPERTIES IMPORTED_LOCATION "${SDL2_mixer_DLL}"
                 IMPORTED_IMPLIB "${SDL2_mixer_LIBRARY}"
    )
  else()
    set_target_properties(
      ${_sdl2_mixer_target_name}
      PROPERTIES IMPORTED_LOCATION "${SDL2_mixer_LIBRARY}"
    )
  endif()

  if(SDL2_mixer_LIBRARY_RELEASE)
    set_property(
      TARGET ${_sdl2_mixer_target_name}
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE
    )
    if(SDL2_mixer_DLL_RELEASE)
      set_target_properties(
        ${_sdl2_mixer_target_name}
        PROPERTIES IMPORTED_LOCATION_RELEASE "${SDL2_mixer_DLL_RELEASE}"
                   IMPORTED_IMPLIB_RELEASE "${SDL2_mixer_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(
        ${_sdl2_mixer_target_name}
        PROPERTIES IMPORTED_LOCATION_RELEASE "${SDL2_mixer_LIBRARY_RELEASE}"
      )
    endif()
  endif()
  if(SDL2_mixer_LIBRARY_DEBUG)
    set_property(
      TARGET ${_sdl2_mixer_target_name}
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG
    )
    if(SDL2_mixer_DLL_DEBUG)
      set_target_properties(
        ${_sdl2_mixer_target_name}
        PROPERTIES IMPORTED_LOCATION_DEBUG "${SDL2_mixer_DLL_DEBUG}"
                   IMPORTED_IMPLIB_DEBUG "${SDL2_mixer_LIBRARY_DEBUG}"
      )
    else()
      set_target_properties(
        ${_sdl2_mixer_target_name}
        PROPERTIES IMPORTED_LOCATION_DEBUG "${SDL2_mixer_LIBRARY_DEBUG}"
      )
    endif()
  endif()

  set(SDL2_mixer_LIBRARIES ${_sdl2_mixer_target_name})
  set(SDL2_mixer_INCLUDE_DIRS "${SDL2_mixer_INCLUDE_DIR}")
endif()

mark_as_advanced(
  SDL2_mixer_INCLUDE_DIR
)
