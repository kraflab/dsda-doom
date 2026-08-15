# Variables defined:
#  SndFile_FOUND
#  SndFile_INCLUDE_DIR
#  SndFile_LIBRARY

find_package(PkgConfig QUIET)
pkg_check_modules(PC_sndfile IMPORTED_TARGET sndfile)

if(PC_sndfile_FOUND)
  if(NOT TARGET SndFile::sndfile)
    add_library(SndFile::sndfile ALIAS PkgConfig::PC_sndfile)
  endif()
  set(SndFile_FOUND TRUE)
  set(SndFile_VERSION ${PC_sndfile_VERSION})
  return()
endif()

find_path(
  SndFile_INCLUDE_DIR sndfile.h
)

find_library(
  SndFile_LIBRARY
  NAMES sndfile libsndfile
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SndFile
  REQUIRED_VARS SndFile_LIBRARY SndFile_INCLUDE_DIR
  VERSION_VAR SndFile_VERSION
)

if(SndFile_FOUND)
  if(NOT TARGET SndFile::sndfile)
    add_library(SndFile::sndfile UNKNOWN IMPORTED)
    set_target_properties(
      SndFile::sndfile
      PROPERTIES IMPORTED_LOCATION "${SndFile_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${SndFile_INCLUDE_DIR}")
  endif()
endif()

mark_as_advanced(SndFile_LIBRARY SndFile_INCLUDE_DIR)
