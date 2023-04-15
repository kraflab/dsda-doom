#[=======================================================================[.rst:
FindSDL2
-------

Finds the SDL2 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SDL2::SDL2``
  The SDL2 shared library (may be an alias of the static)
``SDL2::SDL2-static``
  The SDL2 static library
``SDL2::SDL2main``
  The SDL2main static library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SDL2_FOUND``
  True if the system has the SDL2 package.
``SDL2_SDL2_FOUND``
  True if the system has the SDL2 library (any linkage).
``SDL2_SDL2-static_FOUND``
  True if the system has the SDL2 static library.
``SDL2_SDL2main_FOUND``
  True if the system has the SDL2main library.
``SDL2_INCLUDE_DIRS``
  Include directories needed to use SDL2.
``SDL2_LIBRARIES``
  Libraries needed to link to SDL2.
``SDL2_SDL2_DLL``
  The path to the SDL2 Windows runtime (any config).
``SDL2_SDL2_LIBRARY``
  The path to the SDL2 shared library (any config).
``SDL2_SDL2-static_LIBRARY``
  The path to the SDL2 static library (any config).
``SDL2_SDL2main_LIBRARY``
  The path to the SDL2main library (any config).

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``SDL2_INCLUDE_DIR``
  The directory containing ``SDL2.h``.
``SDL2_SDL2_DLL_RELEASE``
  The path to the SDL2 Windows runtime (release config).
``SDL2_SDL2_DLL_DEBUG``
  The path to the SDL2 Windows runtime (debug config).
``SDL2_SDL2_LIBRARY_RELEASE``
  The path to the SDL2 shared library (release config).
``SDL2_SDL2_LIBRARY_DEBUG``
  The path to the SDL2 shared library (debug config).
``SDL2_SDL2-static_LIBRARY_RELEASE``
  The path to the SDL2 static library (release config).
``SDL2_SDL2-static_LIBRARY_DEBUG``
  The path to the SDL2 static library (debug config).
``SDL2_SDL2main_LIBRARY_RELEASE``
  The path to the SDL2main library (release config).
``SDL2_SDL2main_LIBRARY_DEBUG``
  The path to the SDL2main library (debug config).

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_SDL2 QUIET sdl2)

find_path(
  SDL2_INCLUDE_DIR
  NAMES SDL2/SDL.h
  PATH_SUFFIXES SDL2
  HINTS "${PC_SDL2_INCLUDEDIR}"
)

find_file(
  SDL2_SDL2_DLL_RELEASE
  NAMES SDL2.dll
  PATH_SUFFIXES bin
  HINTS "${PC_SDL2_PREFIX}"
)

find_file(
  SDL2_SDL2_DLL_DEBUG
  NAMES SDL2d.dll
  PATH_SUFFIXES bin
  HINTS "${PC_SDL2_PREFIX}"
)

include(SelectDllConfigurations)
select_dll_configurations(SDL2_SDL2)

if(SDL2_SDL2_DLL)
  set(_sdl2_shared_release_names "SDL2.lib" "libSDL2.dll.a")
  set(_sdl2_static_release_names "SDL2-static.lib")
  set(_sdl2_shared_debug_names "SDL2d.lib")
  set(_sdl2_static_debug_names "SDL2-staticd.lib")
else()
  set(_sdl2_shared_release_names "")
  set(_sdl2_static_release_names "SDL2.lib" "SDL2-static.lib")
  set(_sdl2_shared_debug_names "")
  set(_sdl2_static_debug_names "SDL2d.lib" "SDL2-staticd.lib")
endif()

set(_saved_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".so" ".dylib" ".dll.a")

find_library(
  SDL2_SDL2_LIBRARY_RELEASE
  NAMES ${_sdl2_shared_release_names} SDL2
  HINTS "${PC_SDL2_LIBDIR}"
)

find_library(
  SDL2_SDL2_LIBRARY_DEBUG
  NAMES ${_sdl2_shared_debug_names} SDL2d
  HINTS "${PC_SDL2_LIBDIR}"
)

set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".a")

find_library(
  SDL2_SDL2-static_LIBRARY_RELEASE
  NAMES ${_sdl2_static_release_names} SDL2
  HINTS "${PC_SDL2_LIBDIR}"
)

find_library(
  SDL2_SDL2-static_LIBRARY_DEBUG
  NAMES ${_sdl2_static_debug_names} SDL2d
  HINTS "${PC_SDL2_LIBDIR}"
)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_suffixes})

find_library(
  SDL2_SDL2main_LIBRARY_RELEASE
  NAMES SDL2main
  PATH_SUFFIXES manual-link
  HINTS "${PC_SDL2_LIBDIR}"
)

find_library(
  SDL2_SDL2main_LIBRARY_DEBUG
  NAMES SDL2maind
  PATH_SUFFIXES manual-link
  HINTS "${PC_SDL2_LIBDIR}"
)

include(SelectLibraryConfigurations)
select_library_configurations(SDL2_SDL2)
select_library_configurations(SDL2_SDL2-static)
select_library_configurations(SDL2_SDL2main)

foreach(_component SDL2-static SDL2main)
  if(SDL2_${_component}_LIBRARY)
    set(SDL2_${_component}_FOUND "TRUE")
  else()
    set(SDL2_${_component}_FOUND "FALSE")
  endif()
endforeach()

if(SDL2_SDL2_LIBRARY)
  set(SDL2_SDL2_FOUND "TRUE")
else()
  set(SDL2_SDL2_FOUND "${SDL2_SDL2-static_FOUND}")
endif()

get_flags_from_pkg_config("SHARED" "PC_SDL2" "_sdl2")
get_flags_from_pkg_config("STATIC" "PC_SDL2" "_sdl2_static")


if(SDL2_SDL2-static_FOUND AND NOT PC_SDL2_FOUND)
  if(WIN32)
    set(_sdl2_static_link_libraries user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32)
  elseif(NOT SDL2_SDL2_FOUND)
    set(SDL2_SDL2-static_LINK_LIBRARIES "" CACHE STRING "Additional libraries to link to SDL2-static.")
    set(SDL2_SDL2-static_LINK_DIRECTORIES "" CACHE PATH "Additional directories to search libraries in for SDL2-static.")
    set(_sdl2_static_link_libraries ${SDL2_SDL2-static_LINK_LIBRARIES})
    set(_sdl2_static_link_directories ${SDL2_SDL2-static_LINK_DIRECTORIES})
    if(NOT _sdl2_static_link_libraries)
      message(WARNING
        "pkg-config is unavailable and only a static version of SDL2 was found.\n"
        "Link failures are to be expected.\n"
        "Set `SDL2_SDL2-static_LINK_LIBRARIES` to a list of libraries SDL2 depends on.\n"
        "Set `SDL2_SDL2-static_LINK_DIRECTORIES` to a list of directories to search for libraries in."
      )
    endif()
  endif()
endif()

if(PC_SDL2_FOUND)
  set(SDL2_VERSION "${PC_SDL2_VERSION}")
elseif(EXISTS "${SDL2_INCLUDE_DIR}/SDL_version.h")
  file(READ "${SDL2_INCLUDE_DIR}/SDL_version.h" _sdl_version_h)
  string(REGEX
    MATCH "#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)"
          _sdl2_major_re
          "${_sdl_version_h}")
  set(_sdl2_major "${CMAKE_MATCH_1}")
  string(REGEX
    MATCH "#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)"
          _sdl2_minor_re
          "${_sdl_version_h}")
  set(_sdl2_minor "${CMAKE_MATCH_1}")
  string(REGEX
    MATCH "#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)"
          _sdl2_minor_re
          "${_sdl_version_h}")
  set(_sdl2_minor "${CMAKE_MATCH_1}")
  if(_sdl2_major_re AND _sdl2_minor_re AND _sdl2_minor_re)
    set(${OUT_VAR}
        "${_sdl2_major}.${_sdl2_minor}.${_sdl2_minor}"
        PARENT_SCOPE)
  endif()
endif()

if(SDL2_SDL2_LIBRARY)
  set(_SDL2_LIBRARY "${SDL2_SDL2_LIBRARY}")
else()
  set(_SDL2_LIBRARY "${SDL2_SDL2-static_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SDL2
  REQUIRED_VARS "_SDL2_LIBRARY" "SDL2_INCLUDE_DIR"
  VERSION_VAR "SDL2_VERSION"
  HANDLE_COMPONENTS
)

if(SDL2_FOUND)
  set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}" "${SDL2_INCLUDE_DIR}/..")
  set(SDL2_LIBRARIES SDL2::SDL2)  
endif()

if(SDL2_SDL2_LIBRARY)
  if(NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 SHARED IMPORTED)
    set_target_properties(
      SDL2::SDL2
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
                 INTERFACE_COMPILE_OPTIONS "${_sdl2_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_sdl2_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_sdl2_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_sdl2_link_options}"
    )
  endif()

  if(SDL2_SDL2_DLL)
    set_target_properties(
      SDL2::SDL2
      PROPERTIES IMPORTED_LOCATION "${SDL2_SDL2_DLL}"
                 IMPORTED_IMPLIB "${SDL2_SDL2_LIBRARY}"
    )
  else()
    set_target_properties(
      SDL2::SDL2
      PROPERTIES IMPORTED_LOCATION "${SDL2_SDL2_LIBRARY}"
    )
  endif()

  if(SDL2_SDL2_LIBRARY_RELEASE)
    set_property(
      TARGET SDL2::SDL2
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE
    )
    if(SDL2_SDL2_DLL_RELEASE)
      set_target_properties(
        SDL2::SDL2
        PROPERTIES IMPORTED_LOCATION_RELEASE "${SDL2_SDL2_DLL_RELEASE}"
                   IMPORTED_IMPLIB_RELEASE "${SDL2_SDL2_LIBRARY_RELEASE}"
      )
    else()
      set_target_properties(
        SDL2::SDL2
        PROPERTIES IMPORTED_LOCATION_RELEASE "${SDL2_SDL2_LIBRARY_RELEASE}"
      )
    endif()
  endif()
  if(SDL2_SDL2_LIBRARY_DEBUG)
    set_property(
      TARGET SDL2::SDL2
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG
    )
    if(SDL2_SDL2_DLL_DEBUG)
      set_target_properties(
        SDL2::SDL2
        PROPERTIES IMPORTED_LOCATION_DEBUG "${SDL2_SDL2_DLL_DEBUG}"
                  IMPORTED_IMPLIB_DEBUG "${SDL2_SDL2_LIBRARY_DEBUG}"
      )
    else()
      set_target_properties(
        SDL2::SDL2
        PROPERTIES IMPORTED_LOCATION_DEBUG "${SDL2_SDL2_LIBRARY_DEBUG}"
      )
    endif()
  endif()
endif()

if(SDL2_SDL2-static_FOUND)
  if(NOT SDL2::SDL2-static)
    add_library(SDL2::SDL2-static STATIC IMPORTED)
    set_target_properties(
      SDL2::SDL2-static
      PROPERTIES IMPORTED_LOCATION "${SDL2_SDL2-static_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
                 INTERFACE_COMPILE_OPTIONS "${_sdl2_static_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_sdl2_static_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_sdl2_static_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_sdl2_static_link_options}"
    )
  endif()
  if(SDL2_SDL2-static_LIBRARY_RELEASE)
    set_property(
      TARGET SDL2::SDL2-static
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(
      SDL2::SDL2-static
      PROPERTIES IMPORTED_LOCATION_RELEASE "${SDL2_SDL2-static_LIBRARY_RELEASE}"
    )
  endif()
  if(SDL2_SDL2-static_LIBRARY_DEBUG)
    set_property(
      TARGET SDL2::SDL2-static
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG
    )
    set_target_properties(
      SDL2::SDL2-static
      PROPERTIES IMPORTED_LOCATION_DEBUG "${SDL2_SDL2-static_LIBRARY_DEBUG}"
    )
  endif()

  if(NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 ALIAS SDL2::SDL2-static)
  endif()
endif()

if(SDL2_SDL2main_FOUND)
  set(_sdl2main_link_libraries SDL2::SDL2)
  if(MINGW OR CYGWIN)
    list(APPEND _sdl2main_link_libraries shell32)
  endif()
  if(NOT TARGET SDL2::SDL2main)
    add_library(SDL2::SDL2main STATIC IMPORTED)
    set_target_properties(
      SDL2::SDL2main
      PROPERTIES IMPORTED_LOCATION "${SDL2_SDL2main_LIBRARY}"
                 INTERFACE_LINK_LIBRARIES "${_sdl2main_link_libraries}"
    )
  endif()

  if(SDL2_SDL2main_LIBRARY_RELEASE)
    set_property(
      TARGET SDL2::SDL2main
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(
      SDL2::SDL2main
      PROPERTIES IMPORTED_LOCATION_RELEASE "${SDL2_SDL2main_LIBRARY_RELEASE}"
    )
  endif()
  if(SDL2_SDL2main_LIBRARY_DEBUG)
    set_property(
      TARGET SDL2::SDL2main
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG
    )
    set_target_properties(
      SDL2::SDL2main
      PROPERTIES IMPORTED_LOCATION_DEBUG "${SDL2_SDL2main_LIBRARY_DEBUG}"
    )
  endif()

  list(APPEND SDL2_LIBRARIES SDL2::SDL2main)
endif()

mark_as_advanced(
  SDL2_INCLUDE_DIR
)
