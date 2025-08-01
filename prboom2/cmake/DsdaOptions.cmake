include_guard()

if(WIN32)
  set(default_bin_dir ".")
  set(default_wad_dir ".")
  set(default_copyright_dir ".")
else()
  include(GNUInstallDirs)
  set(default_bin_dir "${CMAKE_INSTALL_BINDIR}")
  set(default_wad_dir "${CMAKE_INSTALL_DATAROOTDIR}/games/doom")
  set(default_copyright_dir "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}")
endif()

set(DSDAPWADDIR "${default_wad_dir}" CACHE STRING "Path to install DSDA-Doom internal WAD")
set(DOOMWADDIR "${CMAKE_INSTALL_PREFIX}/${default_wad_dir}" CACHE PATH "Path to look for WAD files.")
set(DSDA_INSTALL_COPYRIGHT_DIR "${default_copyright_dir}" CACHE STRING "Destination of the copyright file")
set(DSDA_INSTALL_BINDIR "${default_bin_dir}" CACHE STRING "Destination of the dsda-doom binary")

option(ENABLE_PACKAGING "Enable creating CPack packages" ${dsda_is_top_project})
option(STRICT_FIND "Fail configuration if an optional dependency is not found" OFF)
option(SIMPLECHECKS "Enable checks which only impose significant overhead if a posible error is detected" ON)
option(RANGECHECK "Enable internal range checking" OFF)

option(CMAKE_FIND_PACKAGE_PREFER_CONFIG "Search for package config before using Find modules" ON)

if(IS_ABSOLUTE "${DSDAPWADDIR}")
    set(DSDA_ABSOLUTE_PWAD_PATH "${DSDAPWADDIR}")
else()
    set(DSDA_ABSOLUTE_PWAD_PATH "${CMAKE_INSTALL_PREFIX}/${DSDAPWADDIR}")
endif()

# Removed options

if(DEFINED CACHE{ENABLE_LTO})
  message(AUTHOR_WARNING "ENABLE_LTO has been removed, set CMAKE_INTERPROCEDURAL_OPTIMIZATION instead")
endif()
