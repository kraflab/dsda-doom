#[=======================================================================[.rst:
FindSDL2
-------

Finds the SDL2 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SDL2::SDL2``
  The SDL2 library
``SDL2::SDL2main``
  The SDL2 main library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SDL2_FOUND``
  True if the system has the SDL2 package.
``SDL2_SDL2_FOUND``
  True if the system has the SDL2 library.
``SDL2_SDL2main_FOUND``
  True if the system has the SDL2main library.
``SDL2_INCLUDE_DIRS``
  Include directories needed to use SDL2.
``SDL2_LIBRARIES``
  Libraries needed to link to SDL2.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``SDL2_INCLUDE_DIR``
  The directory containing ``SDL2.h``.
``SDL2_SDL2_LIBRARY``
  The path to the SDL2 library.
``SDL2_SDL2main_LIBRARY``
  The path to the SDL2main library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_SDL2 QUIET SDL2)

find_path(
  SDL2_INCLUDE_DIR
  NAMES SDL2/SDL.h
  PATH_SUFFIXES SDL2
  HINTS ${PC_SDL2_INCLUDEDIR})

find_library(
  SDL2_SDL2_LIBRARY_RELEASE
  NAMES SDL2 SDL2-static
  HINTS ${PC_SDL2_LIBDIR})

find_library(
  SDL2_SDL2_LIBRARY_DEBUG
  NAMES SDL2d SDL2-staticd
  HINTS ${PC_SDL2_LIBDIR})

find_library(
  SDL2_SDL2main_LIBRARY_RELEASE
  NAMES SDL2main
  PATH_SUFFIXES manual-link
  HINTS ${PC_SDL2_LIBDIR})

find_library(
  SDL2_SDL2main_LIBRARY_DEBUG
  NAMES SDL2maind
  PATH_SUFFIXES manual-link
  HINTS ${PC_SDL2_LIBDIR})

include(SelectLibraryConfigurations)
select_library_configurations(SDL2_SDL2)
select_library_configurations(SDL2_SDL2main)

foreach(_component SDL2 SDL2main)
  if(SDL2_${_component}_LIBRARY)
    set(SDL2_${_component}_FOUND "TRUE")
  else()
    set(SDL2_${_component}_FOUND "FALSE")
  endif()
endforeach()

if(PC_SDL2_FOUND)
  get_flags_from_pkg_config("${SDL2_LIBRARY}" "PC_SDL2" "_sdl2")
endif()

if(PC_SDL2_FOUND)
  set(SDL2_VERSION "${PC_SDL2_VERSION}")
elseif(EXISTS "${SDL2_INCLUDE_DIR}/SDL_version.h")
  file(READ "${SDL2_INCLUDE_DIR}/SDL_version.h" _sdl_version_h)
  string(REGEX MATCH "#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)"
               _sdl2_major_re "${_sdl_version_h}")
  set(_sdl2_major "${CMAKE_MATCH_1}")
  string(REGEX MATCH "#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)"
               _sdl2_minor_re "${_sdl_version_h}")
  set(_sdl2_minor "${CMAKE_MATCH_1}")
  string(REGEX MATCH "#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)" _sdl2_minor_re
               "${_sdl_version_h}")
  set(_sdl2_minor "${CMAKE_MATCH_1}")
  if(_sdl2_major_re
     AND _sdl2_minor_re
     AND _sdl2_minor_re)
    set(${OUT_VAR}
        "${_sdl2_major}.${_sdl2_minor}.${_sdl2_minor}"
        PARENT_SCOPE)
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SDL2
  REQUIRED_VARS "SDL2_SDL2_LIBRARY" "SDL2_INCLUDE_DIR"
  VERSION_VAR "SDL2_VERSION"
  HANDLE_COMPONENTS)

if(SDL2_SDL2_FOUND)
  set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}" "${SDL2_INCLUDE_DIR}/..")

  if(NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 UNKNOWN IMPORTED)
    set_target_properties(
      SDL2::SDL2
      PROPERTIES IMPORTED_LOCATION "${SDL2_SDL2_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
                 INTERFACE_COMPILE_OPTIONS "${_sdl2_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_sdl2_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_sdl2_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_sdl2_link_options}")
  endif()

  if(SDL2_SDL2_LIBRARY_RELEASE)
    set_property(
      TARGET SDL2::SDL2
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties(SDL2::SDL2 PROPERTIES IMPORTED_LOCATION_RELEASE
                                                "${SDL2_SDL2_LIBRARY_RELEASE}")
  endif()
  if(SDL2_SDL2_LIBRARY_DEBUG)
    set_property(
      TARGET SDL2::SDL2
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(SDL2::SDL2 PROPERTIES IMPORTED_LOCATION_DEBUG
                                                "${SDL2_SDL2_LIBRARY_DEBUG}")
  endif()

  set(SDL2_LIBRARIES SDL2::SDL2)
endif()

if(SDL2_SDL2main_FOUND)
  if(NOT TARGET SDL2::SDL2main)
    add_library(SDL2::SDL2main UNKNOWN IMPORTED)
    set_target_properties(
      SDL2::SDL2main PROPERTIES IMPORTED_LOCATION "${SDL2_SDL2main_LIBRARY}"
                                INTERFACE_LINK_LIBRARIES "SDL2::SDL2")
  endif()

  if(SDL2_SDL2main_LIBRARY_RELEASE)
    set_property(
      TARGET SDL2::SDL2main
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties(
      SDL2::SDL2main PROPERTIES IMPORTED_LOCATION_RELEASE
                                "${SDL2_SDL2main_LIBRARY_RELEASE}")
  endif()
  if(SDL2_SDL2main_LIBRARY_DEBUG)
    set_property(
      TARGET SDL2::SDL2main
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(
      SDL2::SDL2main PROPERTIES IMPORTED_LOCATION_DEBUG
                                "${SDL2_SDL2main_LIBRARY_DEBUG}")
  endif()

  list(APPEND SDL2_LIBRARIES SDL2::SDL2main)
endif()

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_SDL2_LIBRARY SDL2_SDL2main_LIBRARY)
