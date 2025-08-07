#[=======================================================================[.rst:
FindPortMidi
-------

Finds the PortMidi library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``PortMidi::portmidi``
  The PortMidi library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PortMidi_FOUND``
  True if the system has the PortMidi library.
``PortMidi_INCLUDE_DIRS``
  Include directories needed to use PortMidi.
``PortMidi_LIBRARIES``
  Libraries needed to link to PortMidi.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``PortMidi_INCLUDE_DIR``
  The directory containing ``PortMidi.h``.
``PortMidi_DLL``
  The path to the PortMidi Windows runtime.
``PortMidi_LIBRARY``
  The path to the PortMidi library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_portmidi IMPORTED_TARGET portmidi)

if(PC_portmidi_FOUND)
  if(NOT TARGET PortMidi::portmidi)
    add_library(PortMidi::portmidi ALIAS PkgConfig::PC_portmidi)
  endif()
  set(PortMidi_FOUND TRUE)
  set(PortMidi_VERSION ${PC_PortMidi_VERSION})
  return()
endif()

find_path(
  PortMidi_INCLUDE_DIR
  NAMES portmidi.h
)

find_file(
  PortMidi_DLL
  NAMES portmidi.dll libportmidi.dll
  PATH_SUFFIXES bin
)

find_library(
  PortMidi_LIBRARY
  NAMES portmidi
)

if(PortMidi_DLL OR PortMidi_LIBRARY MATCHES ".so|.dylib")
  set(_portmidi_library_type SHARED)
else()
  set(_portmidi_library_type STATIC)
endif()

if(_portmidi_library_type MATCHES "STATIC")
  include(CMakeFindDependencyMacro)
  find_dependency(Threads)
  list(APPEND _portmidi_link_libraries Threads::Threads)
  if(WIN32)
    list(APPEND _portmidi_link_libraries winmm)
  elseif(APPLE)
    list(APPEND _portmidi_link_libraries
         "-Wl,-framework,CoreAudio"
         "-Wl,-framework,CoreFoundation"
         "-Wl,-framework,CoreMIDI"
         "-Wl,-framework,CoreServices"
    )
  elseif(UNIX)
    find_dependency(ALSA)
    list(APPEND _portmidi_link_libraries ALSA::ALSA)
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PortMidi
  REQUIRED_VARS "PortMidi_LIBRARY" "PortMidi_INCLUDE_DIR"
)

if(PortMidi_FOUND)
  if(NOT TARGET PortMidi::portmidi)
    add_library(PortMidi::portmidi ${_portmidi_library_type} IMPORTED)
    set_target_properties(
      PortMidi::portmidi
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${PortMidi_INCLUDE_DIR}"
                 INTERFACE_LINK_LIBRARIES "${_portmidi_link_libraries}"
    )
  endif()

  if (PortMidi_DLL)
    set_target_properties(
      PortMidi::portmidi
      PROPERTIES IMPORTED_LOCATION "${PortMidi_DLL}"
                 IMPORTED_IMPLIB "${PortMidi_LIBRARY}"
    )
  else()
  set_target_properties(
    PortMidi::portmidi
    PROPERTIES IMPORTED_LOCATION "${PortMidi_LIBRARY}"
  )
  endif()

  set(PortMidi_LIBRARIES PortMidi::portmidi)
  set(PortMidi_INCLUDE_DIRS "${PortMidi_INCLUDE_DIR}")
endif()

mark_as_advanced(
  PortMidi_INCLUDE_DIR
  PortMidi_DLL
  PortMidi_LIBRARY
)
