include_guard()

include(DsdaHelpers)

# Internal functions, these should not be called from outside this module

function(dsda_internal_setup_warnings_msvc result_var)
  set(${result_var}
    "/W3"
    PARENT_SCOPE
  )
endfunction()

function(dsda_internal_setup_warnings_gnu result_var)
  set(GNU_WARNINGS
    "-Wall"
    "-Wwrite-strings"
    "-Wundef"
    "-Wtype-limits"
    "-Wcast-qual"
    "-Wpointer-arith"
    "-Wno-unused-function"
    "-Wno-switch"
    "-Wno-sign-compare"
    "-Wno-missing-field-initializers"
  )
  set(GNU_C_WARNINGS
    -Wabsolute-value
    -Wno-pointer-sign
    -Wdeclaration-after-statement
    -Wbad-function-cast
    -Wno-strict-prototypes
  )
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU"
    OR (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 18)
    OR (CMAKE_C_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 17)
  )
    list(APPEND GNU_WARNINGS "-Wno-format-truncation")
  endif()
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    list(APPEND GNU_WARNINGS
      "-Wno-tautological-constant-out-of-range-compare"
      "-Wno-tautological-unsigned-enum-zero-compare"
      "-Wno-misleading-indentation"
    )
  endif()

  set(GNU_WARNINGS_SET ${GNU_WARNINGS} ${GNU_C_WARNINGS})

  include(CheckCCompilerFlag)
  check_c_compiler_flag("${GNU_WARNINGS_SET}" DSDA_SUPPORTS_GNU_WARNINGS)

  if(NOT DSDA_SUPPORTS_GNU_WARNINGS)
    message(AUTHOR_WARNING
      "The default set of warnings is not supported.\n"
      "Set 'DSDA_ENABLED_WARNINGS' manually to a list of warnings you wish to enable."
    )
    set(SUPPORTED_GNU_WARNINGS "")
  else()
    set(SUPPORTED_GNU_WARNINGS ${GNU_WARNINGS})
    foreach(c_warning IN LISTS GNU_C_WARNINGS)
      list(APPEND
        SUPPORTED_GNU_WARNINGS
        $<$<COMPILE_LANGUAGE:C>:${c_warning}>
      )
    endforeach()
  endif()

  set(${result_var}
    "${SUPPORTED_GNU_WARNINGS}"
    PARENT_SCOPE
  )
endfunction()

function(dsda_internal_setup_warnings)
  if(MSVC)
    dsda_internal_setup_warnings_msvc(PLATFORM_WARNINGS)
  else()
    dsda_internal_setup_warnings_gnu(PLATFORM_WARNINGS)
  endif()
  set(DSDA_ENABLED_WARNINGS
    "${PLATFORM_WARNINGS}"
    CACHE STRING
    "List of compiler warnings"
  )
endfunction()

function(dsda_internal_check_fast_math_flag)
  if(MSVC)
    set(FAST_MATH_FLAG "/fp:fast")
  else()
    set(FAST_MATH_FLAG "-ffast-math")
  endif()
  include(CheckCCompilerFlag)
  check_c_compiler_flag("${FAST_MATH_FLAG}" DSDA_SUPPORTS_FAST_MATH)

  if(NOT DSDA_SUPPORTS_FAST_MATH)
    message(AUTHOR_WARNING
      "Could not detect the flag to enable fast math.\n"
      "Set 'DSDA_FAST_MATH_FLAG' manually to a list of flags needed to enable it."
    )
    set(FAST_MATH_FLAG "")
  endif()

  set(DSDA_FAST_MATH_FLAG
    "${FAST_MATH_FLAG}"
    CACHE STRING
    "Flags to enable fast math"
  )
endfunction()

# Public functions

function(dsda_target_set_warnings tgt)
  dsda_fail_if_invalid_target(${tgt})

  if(NOT DEFINED CACHE{DSDA_ENABLED_WARNINGS})
    dsda_internal_setup_warnings()
  endif()

  target_compile_options(${tgt}
    PRIVATE
    ${DSDA_ENABLED_WARNINGS}
  )
endfunction()

function(dsda_target_silence_deprecation tgt)
  dsda_fail_if_invalid_target(${tgt})

  if(WIN32)
    target_compile_definitions(${tgt}
      PRIVATE
      "_CRT_SECURE_NO_WARNINGS" # Warnings for not using "secure" (_s) functions
      "_CRT_NONSTDC_NO_WARNINGS" # Warnings for using POSIX names of functions
    )
  elseif(APPLE)
    target_compile_definitions(${tgt}
      PRIVATE
      "GL_SILENCE_DEPRECATION" # Calling GL functions is deprecated on macOS
    )
  endif()
endfunction()

function(dsda_target_enable_fast_math tgt)
  dsda_fail_if_invalid_target(${tgt})

  if(NOT DEFINED CACHE{DSDA_FAST_MATH_FLAG})
    dsda_internal_check_fast_math_flag()
  endif()

  target_compile_options(${tgt}
    PRIVATE
    ${DSDA_FAST_MATH_FLAG}
  )
endfunction()
