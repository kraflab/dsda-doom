include_guard()

function(dsda_fail_if_invalid_target tgt)
  if(NOT TARGET ${tgt})
    message(FATAL_ERROR "${tgt} is not a valid CMake target.")
  endif()
endfunction()

function(dsda_set_default_build_config build_config)
  get_cmake_property(is_multi_config GENERATOR_IS_MULTI_CONFIG)
  if(NOT is_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "${build_config}"
      CACHE STRING "Build configuration"
      FORCE
    )
    set_property(
      CACHE CMAKE_BUILD_TYPE
      PROPERTY STRINGS
      "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
  elseif(CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    if(build_config IN_LIST CMAKE_CONFIGURATION_TYPES)
      set(CMAKE_DEFAULT_BUILD_TYPE ${build_config})
    else()
      message(AUTHOR_WARNING "${build_config} is not a known config type.")
    endif()
  endif()
endfunction()
