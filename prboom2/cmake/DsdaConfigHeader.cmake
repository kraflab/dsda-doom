include_guard()

include(DsdaHelpers)

set(DSDA_INTERNAL_GENERATED_CONFIG_DIR "${PROJECT_BINARY_DIR}/build-config")

# Internal functions, these should not be called from outside this module

function(dsda_internal_check_symbols)
  include(CheckSymbolExists)

  check_symbol_exists(stricmp "string.h" HAVE_STRICMP)
  check_symbol_exists(strnicmp "string.h" HAVE_STRNICMP)
  check_symbol_exists(getopt "unistd.h" HAVE_GETOPT)
  check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
  check_symbol_exists(CreateFileMapping "windows.h" HAVE_CREATE_FILE_MAPPING)
  check_symbol_exists(strsignal "string.h" HAVE_STRSIGNAL)
  check_symbol_exists(mkstemp "stdlib.h" HAVE_MKSTEMP)
  check_symbol_exists(getpwuid "unistd.h;sys/types.h;pwd.h" HAVE_GETPWUID)
endfunction()

function(dsda_internal_check_includes)
  include(CheckIncludeFile)

  check_include_file("sys/wait.h" HAVE_SYS_WAIT_H)
  check_include_file("unistd.h" HAVE_UNISTD_H)
  check_include_file("asm/byteorder.h" HAVE_ASM_BYTEORDER_H)
  check_include_file("dirent.h" HAVE_DIRENT_H)
endfunction()

function(dsda_internal_check_variables)
  set(expected_vars
    PROJECT_NAME
    PROJECT_TARNAME
    WAD_DATA
    PROJECT_VERSION
    PROJECT_STRING
    DOOMWADDIR
    DSDA_ABSOLUTE_PWAD_PATH
    WORDS_BIGENDIAN
    SIMPLECHECKS
    RANGECHECK
  )
  foreach(var IN LISTS expected_vars)
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "config.h cannot be generated: \"${var}\" is not set")
    endif()
  endforeach()
endfunction()

function(dsda_internal_generate_build_config)
  include(CheckBigEndian)
  check_big_endian(WORDS_BIGENDIAN)

  dsda_internal_check_symbols()
  dsda_internal_check_includes()
  dsda_internal_check_variables()

  configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/config.h.cin" 
    "${DSDA_INTERNAL_GENERATED_CONFIG_DIR}/config.h"
  )
endfunction()

dsda_internal_generate_build_config()

# Public functions

function(dsda_target_use_config_h tgt)
  dsda_fail_if_invalid_target(${tgt})

  target_sources(${tgt}
    PRIVATE
    "${DSDA_INTERNAL_GENERATED_CONFIG_DIR}/config.h"
  )

  target_include_directories(${tgt}
    PRIVATE
    $<BUILD_INTERFACE:${DSDA_INTERNAL_GENERATED_CONFIG_DIR}>
  )

  target_compile_definitions(${TARGET}
    PRIVATE
    HAVE_CONFIG_H
    $<$<NOT:$<BOOL:${HAVE_STRICMP}>>:stricmp=strcasecmp>
    $<$<NOT:$<BOOL:${HAVE_STRNICMP}>>:strnicmp=strncasecmp>
  )
endfunction()
