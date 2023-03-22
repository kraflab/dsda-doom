#[=======================================================================[.rst:
FindSDL2_mixer
-------

Finds the SDL2_mixer library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SDL2_mixer::SDL2_mixer``
  The SDL2_mixer library

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
``SDL2_mixer_LIBRARY``
  The path to the SDL2_mixer library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_SDL2_MIXER QUIET SDL2_mixer)

find_path(
  SDL2_mixer_INCLUDE_DIR
  NAMES SDL_mixer.h
  PATH_SUFFIXES SDL2
  HINTS ${PC_SDL2_MIXER_INCLUDEDIR})

find_library(
  SDL2_mixer_LIBRARY_RELEASE
  NAMES SDL2_mixer SDL2_mixer-static
  HINTS ${PC_SDL2_MIXER_LIBDIR})

find_library(
  SDL2_mixer_LIBRARY_DEBUG
  NAMES SDL2_mixerd SDL2_mixer-staticd
  HINTS ${PC_SDL2_MIXER_LIBDIR})

include(SelectLibraryConfigurations)
select_library_configurations(SDL2_mixer)

if(PC_SDL2_MIXER_FOUND)
  get_flags_from_pkg_config("${SDL2_mixer_LIBRARY}" "PC_SDL2_MIXER"
                            "_sdl2_mixer")
elseif(SDL2_mixer_LIBRARY)
  set(_sdl2_mixer_link_libraries
      ""
      CACHE FILEPATH "Additional libraries to link to SDL2_mixer.")
  if(NOT _sdl2_mixer_link_libraries)
    message(
      "pkg-config is unavailable, if SDL2_mixer is a static library, linking will fail.\n"
      "Set '_sdl2_mixer_link_libraries' to the list of libraries to link.")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SDL2_mixer REQUIRED_VARS "SDL2_mixer_LIBRARY" "SDL2_mixer_INCLUDE_DIR")

if(SDL2_mixer_FOUND)
  if(NOT TARGET SDL2_mixer::SDL2_mixer)
    add_library(SDL2_mixer::SDL2_mixer UNKNOWN IMPORTED)
    set_target_properties(
      SDL2_mixer::SDL2_mixer
      PROPERTIES IMPORTED_LOCATION "${SDL2_mixer_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${SDL2_mixer_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_sdl2_mixer_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_sdl2_mixer_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_sdl2_mixer_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_sdl2_mixer_link_options}")
  endif()

  if(SDL2_mixer_LIBRARY_RELEASE)
    set_property(
      TARGET SDL2_mixer::SDL2_mixer
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties(
      SDL2_mixer::SDL2_mixer PROPERTIES IMPORTED_LOCATION_RELEASE
                                        "${SDL2_mixer_LIBRARY_RELEASE}")
  endif()
  if(SDL2_mixer_LIBRARY_DEBUG)
    set_property(
      TARGET SDL2_mixer::SDL2_mixer
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(
      SDL2_mixer::SDL2_mixer PROPERTIES IMPORTED_LOCATION_DEBUG
                                        "${SDL2_mixer_LIBRARY_DEBUG}")
  endif()

  set(SDL2_mixer_LIBRARIES SDL2_mixer::SDL2_mixer)
  set(SDL2_mixer_INCLUDE_DIRS "${SDL2_mixer_INCLUDE_DIR}")
endif()

mark_as_advanced(SDL2_mixer_INCLUDE_DIR SDL2_mixer_LIBRARY)
