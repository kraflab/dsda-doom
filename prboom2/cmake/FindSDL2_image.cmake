#[=======================================================================[.rst:
FindSDL2_image
-------

Finds the SDL2_image library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SDL2_image::SDL2_image``
  The SDL2_image library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SDL2_image_FOUND``
  True if the system has the SDL2_image library.
``SDL2_image_INCLUDE_DIRS``
  Include directories needed to use SDL2_image.
``SDL2_image_LIBRARIES``
  Libraries needed to link to SDL2_image.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``SDL2_image_INCLUDE_DIR``
  The directory containing ``SDL2_image.h``.
``SDL2_image_LIBRARY``
  The path to the SDL2_image library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_SDL2_IMAGE QUIET SDL2_image)

find_path(
  SDL2_image_INCLUDE_DIR
  NAMES SDL_image.h
  PATH_SUFFIXES SDL2
  HINTS ${PC_SDL2_IMAGE_INCLUDEDIR})

find_library(
  SDL2_image_LIBRARY_RELEASE
  NAMES SDL2_image SDL2_image-static
  HINTS ${PC_SDL2_IMAGE_LIBDIR})

find_library(
  SDL2_image_LIBRARY_DEBUG
  NAMES SDL2_imaged SDL2_image-staticd
  HINTS ${PC_SDL2_IMAGE_LIBDIR})

include(SelectLibraryConfigurations)
select_library_configurations(SDL2_image)

if(PC_SDL2_IMAGE_FOUND)
  get_flags_from_pkg_config("${SDL2_image_LIBRARY}" "PC_SDL2_IMAGE"
                            "_sdl2_image")
elseif(SDL2_image_LIBRARY)
  set(_sdl2_image_link_libraries
      ""
      CACHE FILEPATH "Additional libraries to link to SDL2_image.")
  if(NOT _sdl2_image_link_libraries)
    message(
      "pkg-config is unavailable, if SDL2_image is a static library, linking will fail.\n"
      "Set '_sdl2_image_link_libraries' to the list of libraries to link.")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SDL2_image REQUIRED_VARS "SDL2_image_LIBRARY" "SDL2_image_INCLUDE_DIR")

if(SDL2_image_FOUND)
  if(NOT TARGET SDL2_image::SDL2_image)
    add_library(SDL2_image::SDL2_image UNKNOWN IMPORTED)
    set_target_properties(
      SDL2_image::SDL2_image
      PROPERTIES IMPORTED_LOCATION "${SDL2_image_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${SDL2_image_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_sdl2_image_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_sdl2_image_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_sdl2_image_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_sdl2_image_link_options}")
  endif()

  if(SDL2_image_LIBRARY_RELEASE)
    set_property(
      TARGET SDL2_image::SDL2_image
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties(
      SDL2_image::SDL2_image PROPERTIES IMPORTED_LOCATION_RELEASE
                                        "${SDL2_image_LIBRARY_RELEASE}")
  endif()
  if(SDL2_image_LIBRARY_DEBUG)
    set_property(
      TARGET SDL2_image::SDL2_image
      APPEND
      PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(
      SDL2_image::SDL2_image PROPERTIES IMPORTED_LOCATION_DEBUG
                                        "${SDL2_image_LIBRARY_DEBUG}")
  endif()

  set(SDL2_image_LIBRARIES SDL2_image::SDL2_image)
  set(SDL2_image_INCLUDE_DIRS SDL2_image::SDL2_image)
endif()

mark_as_advanced(SDL2_image_INCLUDE_DIR SDL2_image_LIBRARY)
