function(clean_flag_name flag outvar)
  string(TOUPPER "${flag}" "_upper_name")
  string(REGEX REPLACE "[-/:]" "_" "_cleaned_name" "${_upper_name}")
  set("${outvar}"
      "${_cleaned_name}"
      PARENT_SCOPE)
endfunction()

function(check_flags_list flags_list outvar)
  include(CheckCCompilerFlag)
  foreach(_flag IN LISTS flags_list)
    clean_flag_name("${_flag}" _flag_name)
    check_c_compiler_flag("${_flag}" "HAVE${_flag_name}")
    if(HAVE${_flag_name})
      list(APPEND _supported_flags "${_flag}")
    endif()
  endforeach()
  set("${outvar}"
      "${_supported_flags}"
      PARENT_SCOPE)
endfunction()

# Sets `ouvar` to a list of warnings supported by the compiler MSVC (and
# clang-cl) skip the checks and use /W3
function(get_supported_warnings outvar)
  if(MSVC)
    set("${outvar}" "/W3")
    return()
  endif()

  set(_warnings
      "-Wall"
      "-Wwrite-strings"
      "-Wundef"
      "-Wtype-limits"
      "-Wcast-qual"
      "-Wpointer-arith"
      "-Wno-unused-function"
      "-Wno-switch"
      "-Wno-sign-compare"
      "-Wno-format-truncation"
      "-Wno-missing-field-initializers")
  check_flags_list("${_warnings}" "_supported_warnings")

  # The following warnings do not apply to C++ and should be treated separately
  set(_c_only_warnings "-Wabsolute-value" "-Wno-pointer-sign"
                       "-Wdeclaration-after-statement" "-Wbad-function-cast")
  check_flags_list("${_c_only_warnings}" "_supported_c_warnings")
  foreach(_c_warning IN LISTS _supported_c_warnings)
    list(APPEND _supported_warnings $<$<COMPILE_LANGUAGE:C>:${_c_warning}>)
  endforeach()

  set("${outvar}"
      "${_supported_warnings}"
      PARENT_SCOPE)
endfunction()

# Sets `outvar` to the flag needed to enable fast math
function(get_fast_math_flag outvar)
  set(_fast_math_flags "-ffast-math" "/fp:fast")
  check_flags_list("${_fast_math_flags}" "_supported_flags")
  set("${outvar}"
      "${_supported_flags}"
      PARENT_SCOPE)
endfunction()

# Sets `outvar` to the list of compile definitions necessary to silence
# deprecation warnings
function(get_deprecation_silencing_definitions outvar)
  if(WIN32) # The clang (non-cl) driver also trigger these warnings on Windows
    set(_no_deprecation_definitions "_CRT_SECURE_NO_WARNINGS"
                                    "_CRT_NONSTDC_NO_WARNINGS")
  else()
    set(_no_deprecation_definitions "")
  endif()
  set("${outvar}"
      "${_no_deprecation_definitions}"
      PARENT_SCOPE)
endfunction()

# Sets `outvar` to the list of compile definitions used by the dsda-doom
# executable
function(get_compile_definitions outvar)
  set(_required_definitions "HAVE_CONFIG_H")

  if(NOT HAVE_STRICMP)
    list(APPEND _required_definitions "stricmp=strcasecmp")
  endif()
  if(NOT HAVE_STRNICMP)
    list(APPEND _required_definitions "strnicmp=strncasecmp")
  endif()

  set("${outvar}"
      "${_required_definitions}"
      PARENT_SCOPE)
endfunction()
