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

find_package(PkgConfig QUIET)
pkg_check_modules(PC_vorbis IMPORTED_TARGET vorbis)
pkg_check_modules(PC_vorbisenc IMPORTED_TARGET vorbisenc)
pkg_check_modules(PC_vorbisfile IMPORTED_TARGET vorbisfile)

if(PC_vorbis_FOUND AND PC_vorbisenc_FOUND AND PC_vorbisfile_FOUND)
  if(NOT TARGET Vorbis::vorbis)
    add_library(Vorbis::vorbis ALIAS PkgConfig::PC_vorbis)
    set(Vorbis_Vorbis_FOUND TRUE)
  endif()
  if(NOT TARGET Vorbis::vorbisenc)
    add_library(Vorbis::vorbisenc ALIAS PkgConfig::PC_vorbisenc)
    set(Vorbis_Enc_FOUND TRUE)
  endif()
  if(NOT TARGET Vorbis::vorbisfile)
    add_library(Vorbis::vorbisfile ALIAS PkgConfig::PC_vorbisfile)
    set(Vorbis_File_FOUND TRUE)
  endif()
  set(Vorbis_FOUND TRUE)
  set(Vorbis_VERSION ${PC_Vorbis_VERSION})
  return()
endif()

find_path(
  Vorbis_INCLUDE_DIR
  NAMES vorbis/codec.h
)

find_file(
  Vorbis_Vorbis_DLL
  NAMES vorbis.dll libvorbis.dll libvorbis-0.dll
  PATH_SUFFIXES bin
)

find_file(
  Vorbis_Enc_DLL
  NAMES vorbisenc.dll libvorbisenc.dll libvorbisenc-2.dll
  PATH_SUFFIXES bin
)

find_file(
  Vorbis_File_DLL
  NAMES vorbisfile.dll libvorbisfile.dll libvorbisfile-3.dll
  PATH_SUFFIXES bin
)

find_library(
  Vorbis_Vorbis_LIBRARY
  NAMES vorbis
)

find_library(
  Vorbis_Enc_LIBRARY
  NAMES vorbisenc
)

find_library(
  Vorbis_File_LIBRARY
  NAMES vorbisfile
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

find_library(_has_math_lib NAMES m)
if(_has_math_lib)
  list(APPEND _vorbis_link_libraries m)
endif()
include(CMakeFindDependencyMacro)
find_dependency(Ogg)
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
               INTERFACE_LINK_LIBRARIES "${_vorbis_link_libraries}"
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
               INTERFACE_LINK_LIBRARIES "${_vorbisenc_link_libraries}"
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
               INTERFACE_LINK_LIBRARIES "${_vorbisfile_link_libraries}"
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
