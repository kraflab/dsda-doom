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
``PortMidi_LIBRARY``
  The path to the PortMidi library.

#]=======================================================================]

if(PortMidi_FIND_REQUIRED)
  set(_find_package_search_type "REQUIRED")
elseif(PortMidi_FIND_QUIETLY)
  set(_find_package_search_type "QUIET")
endif()

find_package(PkgConfig QUIET)
pkg_check_modules(PC_PORTMIDI QUIET portmidi)

find_path(
  PortMidi_INCLUDE_DIR
  NAMES portmidi.h
  HINTS ${PC_PORTMIDI_INCLUDEDIR})

find_library(
  PortMidi_LIBRARY
  NAMES portmidi
  HINTS ${PC_PORTMIDI_LIBDIR})

if(PC_PORTMIDI_FOUND)
  get_flags_from_pkg_config("${PortMidi_LIBRARY}" "PC_PORTMIDI" "_portmidi")
else()
  find_package(Threads ${_find_package_search_type})
  list(APPEND _portmidi_link_libraries Threads::Threads)
  if(WIN32)
    list(APPEND _portmidi_link_libraries winmm)
  elseif(APPLE)
    list(APPEND _portmidi_link_libraries "-Wl,-framework,CoreAudio"
         "-Wl,-framework,CoreFoundation" "-Wl,-framework,CoreMIDI"
         "-Wl,-framework,CoreServices")
  elseif(UNIX)
    find_package(ALSA ${_find_package_search_type})
    list(APPEND _portmidi_link_libraries ALSA::ALSA)
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PortMidi REQUIRED_VARS "PortMidi_LIBRARY"
                                                         "PortMidi_INCLUDE_DIR")

if(PortMidi_FOUND)
  if(NOT TARGET PortMidi::portmidi)
    add_library(PortMidi::portmidi UNKNOWN IMPORTED)
    set_target_properties(
      PortMidi::portmidi
      PROPERTIES IMPORTED_LOCATION "${PortMidi_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${PortMidi_INCLUDE_DIR}"
                 INTERFACE_COMPILE_OPTIONS "${_portmidi_compile_options}"
                 INTERFACE_LINK_LIBRARIES "${_portmidi_link_libraries}"
                 INTERFACE_LINK_DIRECTORIES "${_portmidi_link_directories}"
                 INTERFACE_LINK_OPTIONS "${_portmidi_link_options}")
  endif()

  set(PortMidi_LIBRARIES PortMidi::portmidi)
  set(PortMidi_INCLUDE_DIRS PortMidi::portmidi)
endif()

mark_as_advanced(PortMidi_INCLUDE_DIR PortMidi_LIBRARY)
