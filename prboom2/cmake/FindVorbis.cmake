#[=======================================================================[.rst:
FindVorbis
-------

Finds the Vorbis libraries.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Vorbis::vorbis``
  The base encoder and decoder library
``Vorbis::vorbisenc``
  The high-level encoder library
``Vorbis::vorbisfile``
  The high-level decoder library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Vorbis_FOUND``
  True if the system has all the requested components.
``Vorbis_Vorbis_FOUND``
  True if vorbis is found
``Vorbis_Enc_FOUND``
  True if vorbisenc is found
``Vorbis_File_FOUND``
  True if vorbisfile is found

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Vorbis_INCLUDE_DIR``
  The directory containing ``vorbis/vorbis.h``.
``Vorbis_Vorbis_LIBRARY``
  The path to the vorbis library.
``Vorbis_Enc_LIBRARY``
  The path to the vorbisenc library.
``Vorbis_File_LIBRARY``
  The path to the vorbisfile library.

#]=======================================================================]

if(Vorbis_FIND_REQUIRED)
  set(_find_package_search_type "REQUIRED")
elseif(Vorbis_FIND_QUIETLY)
  set(_find_package_search_type "QUIET")
endif()

find_package(PkgConfig QUIET)
pkg_check_modules(PC_VORBIS QUIET vorbis)
pkg_check_modules(PC_VORBISENC QUIET vorbisenc)
pkg_check_modules(PC_VORBISFILE QUIET vorbisfile)

find_path(
  Vorbis_INCLUDE_DIR
  NAMES vorbis/codec.h
  HINTS ${PC_VORBIS_INCLUDEDIR})

find_library(
  Vorbis_Vorbis_LIBRARY
  NAMES vorbis
  HINTS ${PC_VORBIS_LIBDIR})

find_library(
  Vorbis_Enc_LIBRARY
  NAMES vorbisenc
  HINTS ${PC_VORBISENC_LIBDIR})

find_library(
  Vorbis_File_LIBRARY
  NAMES vorbisfile
  HINTS ${PC_VORBISFILE_LIBDIR})

if(PC_VORBIS_FOUND)
  get_flags_from_pkg_config("${Vorbis_Vorbis_LIBRARY}" "PC_VORBIS" "_vorbis")
endif()

if(PC_VORBISENC_FOUND)
  get_flags_from_pkg_config("${Vorbis_Enc_LIBRARY}" "PC_VORBISENC" "_vorbisenc")
endif()

if(PC_VORBISFILE_FOUND)
  get_flags_from_pkg_config("${Vorbis_File_LIBRARY}" "PC_VORBISFILE"
                            "_vorbisfile")
endif()

find_library(_has_math_lib NAMES m)
if(_has_math_lib)
  list(APPEND _vorbis_link_libraries m)
endif()
find_package(Ogg ${_find_package_search_type})
list(APPEND _vorbis_link_libraries Ogg::ogg)
set(_vorbisenc_link_libraries Vorbis::vorbis)
set(_vorbisfile_link_libraries Vorbis::vorbis)

if(Vorbis_Vorbis_LIBRARY)
  set(Vorbis_Vorbis_FOUND "TRUE")
else()
  set(Vorbis_Vorbis_FOUND "FALSE")
endif()

if(Vorbis_Enc_LIBRARY)
  set(Vorbis_Enc_FOUND "TRUE")
else()
  set(Vorbis_Enc_FOUND "FALSE")
endif()

if(Vorbis_File_LIBRARY)
  set(Vorbis_File_FOUND "TRUE")
else()
  set(Vorbis_File_FOUND "FALSE")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Vorbis
  REQUIRED_VARS "Vorbis_File_LIBRARY" "Vorbis_Vorbis_LIBRARY"
                "Vorbis_INCLUDE_DIR"
  HANDLE_COMPONENTS)

if(Vorbis_Vorbis_FOUND AND NOT TARGET Vorbis::vorbis)
  add_library(Vorbis::vorbis UNKNOWN IMPORTED)
  set_target_properties(
    Vorbis::vorbis
    PROPERTIES IMPORTED_LOCATION "${Vorbis_Vorbis_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_INCLUDE_DIR}"
               INTERFACE_COMPILE_OPTIONS "${_vorbis_compile_options}"
               INTERFACE_LINK_LIBRARIES "${_vorbis_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_vorbis_link_directories}"
               INTERFACE_LINK_OPTIONS "${_vorbis_link_options}")
endif()

if(Vorbis_Enc_FOUND AND NOT TARGET Vorbis::vorbisenc)
  add_library(Vorbis::vorbisenc UNKNOWN IMPORTED)
  set_target_properties(
    Vorbis::vorbisenc
    PROPERTIES IMPORTED_LOCATION "${Vorbis_Enc_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_INCLUDE_DIR}"
               INTERFACE_COMPILE_OPTIONS "${_vorbisenc_compile_options}"
               INTERFACE_LINK_LIBRARIES "${_vorbisenc_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_vorbisenc_link_directories}"
               INTERFACE_LINK_OPTIONS "${_vorbisenc_link_options}")
endif()

if(Vorbis_File_FOUND AND NOT TARGET Vorbis::vorbisfile)
  add_library(Vorbis::vorbisfile UNKNOWN IMPORTED)
  set_target_properties(
    Vorbis::vorbisfile
    PROPERTIES IMPORTED_LOCATION "${Vorbis_File_LIBRARY}"
               INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_INCLUDE_DIR}"
               INTERFACE_COMPILE_OPTIONS "${_vorbisfile_compile_options}"
               INTERFACE_LINK_LIBRARIES "${_vorbisfile_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_vorbisfile_link_directories}"
               INTERFACE_LINK_OPTIONS "${_vorbisfile_link_options}")
endif()

mark_as_advanced(Vorbis_INCLUDE_DIR Vorbis_Vorbis_LIBRARY Vorbis_Enc_LIBRARY
                 Vorbis_File_LIBRARY)
