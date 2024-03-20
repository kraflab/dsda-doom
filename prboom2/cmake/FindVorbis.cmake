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
``Vorbis_Vorbis_DLL``
  The path to the vorbis Windows runtime.
``Vorbis_Enc_DLL``
  The path to the vorbisenc Windows runtime.
``Vorbis_File_DLL``
  The path to the vorbisfile Windows runtime.
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
else()
  set(_find_package_search_type "")
endif()

find_package(PkgConfig QUIET)
pkg_check_modules(PC_VORBIS QUIET vorbis)
pkg_check_modules(PC_VORBISENC QUIET vorbisenc)
pkg_check_modules(PC_VORBISFILE QUIET vorbisfile)

find_path(
  Vorbis_INCLUDE_DIR
  NAMES vorbis/codec.h
  HINTS "${PC_VORBIS_INCLUDEDIR}"
)

find_file(
  Vorbis_Vorbis_DLL
  NAMES vorbis.dll libvorbis.dll libvorbis-0.dll
  PATH_SUFFIXES bin
  HINTS "${PC_VORBIS_PREFIX}"
)

find_file(
  Vorbis_Enc_DLL
  NAMES vorbisenc.dll libvorbisenc.dll libvorbisenc-2.dll
  PATH_SUFFIXES bin
  HINTS "${PC_VORBISENC_PREFIX}"
)

find_file(
  Vorbis_File_DLL
  NAMES vorbisfile.dll libvorbisfile.dll libvorbisfile-3.dll 
  PATH_SUFFIXES bin
  HINTS "${PC_VORBISFILE_PREFIX}"
)

find_library(
  Vorbis_Vorbis_LIBRARY
  NAMES vorbis
  HINTS "${PC_VORBIS_LIBDIR}"
)

find_library(
  Vorbis_Enc_LIBRARY
  NAMES vorbisenc
  HINTS "${PC_VORBISENC_LIBDIR}"
)

find_library(
  Vorbis_File_LIBRARY
  NAMES vorbisfile
  HINTS "${PC_VORBISFILE_LIBDIR}"
)

if(Vorbis_Vorbis_DLL OR Vorbis_Vorbis_LIBRARY MATCHES ".so|.dylib")
  set(_vorbis_library_type SHARED)
else()
  set(_vorbis_library_type STATIC)
endif()

if(Vorbis_Enc_DLL OR Vorbis_Enc_LIBRARY MATCHES ".so|.dylib")
  set(_vorbisenc_library_type SHARED)
else()
  set(_vorbisenc_library_type STATIC)
endif()

if(Vorbis_File_DLL OR Vorbis_File_LIBRARY MATCHES ".so|.dylib")
  set(_vorbisfile_library_type SHARED)
else()
  set(_vorbisfile_library_type STATIC)
endif()

get_flags_from_pkg_config("${_vorbis_library_type}" "PC_VORBIS" "_vorbis")
get_flags_from_pkg_config("${_vorbisenc_library_type}" "PC_VORBISENC" "_vorbisenc")
get_flags_from_pkg_config("${_vorbisfile_library_type}" "PC_VORBISFILE" "_vorbisfile")

find_library(_has_math_lib NAMES m)
if(_has_math_lib)
  list(APPEND _vorbis_link_libraries m)
endif()
find_package(Ogg ${_find_package_search_type})
list(APPEND _vorbis_link_libraries Ogg::ogg)
set(_vorbisenc_link_libraries Vorbis::vorbis)
set(_vorbisfile_link_libraries Vorbis::vorbis)

foreach(_component "Vorbis" "Enc" "File")  
  if(Vorbis_${_component}_LIBRARY)
    set(Vorbis_${_component}_FOUND "TRUE")
  else()
    set(Vorbis_${_component}_FOUND "FALSE")
  endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Vorbis
  REQUIRED_VARS "Vorbis_File_LIBRARY"
                "Vorbis_Vorbis_LIBRARY"
                "Vorbis_INCLUDE_DIR"
  HANDLE_COMPONENTS
)

if(Vorbis_Vorbis_FOUND AND NOT TARGET Vorbis::vorbis)
  add_library(Vorbis::vorbis ${_vorbis_library_type} IMPORTED)
  set_target_properties(
    Vorbis::vorbis
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_INCLUDE_DIR}"
               INTERFACE_COMPILE_OPTIONS "${_vorbis_compile_options}"
               INTERFACE_LINK_LIBRARIES "${_vorbis_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_vorbis_link_directories}"
               INTERFACE_LINK_OPTIONS "${_vorbis_link_options}"
  )
  if(Vorbis_Vorbis_DLL)
    set_target_properties(
      Vorbis::vorbis
      PROPERTIES IMPORTED_LOCATION "${Vorbis_Vorbis_DLL}"
                 IMPORTED_IMPLIB "${Vorbis_Vorbis_LIBRARY}"
    )
  else()
    set_target_properties(
      Vorbis::vorbis
      PROPERTIES IMPORTED_LOCATION "${Vorbis_Vorbis_LIBRARY}"
    )
  endif()
endif()

if(Vorbis_Enc_FOUND AND NOT TARGET Vorbis::vorbisenc)
  add_library(Vorbis::vorbisenc ${_vorbisenc_library_type} IMPORTED)
  set_target_properties(
    Vorbis::vorbisenc
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_INCLUDE_DIR}"
               INTERFACE_COMPILE_OPTIONS "${_vorbisenc_compile_options}"
               INTERFACE_LINK_LIBRARIES "${_vorbisenc_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_vorbisenc_link_directories}"
               INTERFACE_LINK_OPTIONS "${_vorbisenc_link_options}"
  )
  if(Vorbis_Enc_DLL)
    set_target_properties(
      Vorbis::vorbisenc
      PROPERTIES IMPORTED_LOCATION "${Vorbis_Enc_DLL}"
                 IMPORTED_IMPLIB "${Vorbis_Enc_LIBRARY}"
    )
  else()
    set_target_properties(
      Vorbis::vorbisenc
      PROPERTIES IMPORTED_LOCATION "${Vorbis_Enc_LIBRARY}"
    )
  endif()
endif()

if(Vorbis_File_FOUND AND NOT TARGET Vorbis::vorbisfile)
  add_library(Vorbis::vorbisfile ${_vorbisfile_library_type} IMPORTED)
  set_target_properties(
    Vorbis::vorbisfile
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_INCLUDE_DIR}"
               INTERFACE_COMPILE_OPTIONS "${_vorbisfile_compile_options}"
               INTERFACE_LINK_LIBRARIES "${_vorbisfile_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_vorbisfile_link_directories}"
               INTERFACE_LINK_OPTIONS "${_vorbisfile_link_options}"
  )
  if(Vorbis_File_DLL)
    set_target_properties(
      Vorbis::vorbisfile
      PROPERTIES IMPORTED_LOCATION "${Vorbis_File_DLL}"
                 IMPORTED_IMPLIB "${Vorbis_File_LIBRARY}"
    )
  else()
    set_target_properties(
      Vorbis::vorbisfile
      PROPERTIES IMPORTED_LOCATION "${Vorbis_File_LIBRARY}"
    )
  endif()
endif()

mark_as_advanced(
  Vorbis_INCLUDE_DIR
  Vorbis_Vorbis_DLL
  Vorbis_Enc_DLL
  Vorbis_File_DLL
  Vorbis_Vorbis_LIBRARY
  Vorbis_Enc_LIBRARY
  Vorbis_File_LIBRARY
)
