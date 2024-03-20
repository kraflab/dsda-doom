# Similar to CMake's SelectLibraryConfigurations but adapted to DLLs

macro(select_dll_configurations basename)
  if(NOT ${basename}_DLL_RELEASE)
    set(${basename}_DLL_RELEASE
        "${basename}_DLL_RELEASE-NOTFOUND"
        CACHE FILEPATH "Path to the release DLL.")
  endif()
  if(NOT ${basename}_DLL_DEBUG)
    set(${basename}_DLL_DEBUG
        "${basename}_DLL_DEBUG-NOTFOUND"
        CACHE FILEPATH "Path to the debug DLL.")
  endif()

  get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(${basename}_DLL_DEBUG
     AND ${basename}_DLL_RELEASE
     AND NOT ${basename}_DLL_DEBUG STREQUAL ${basename}_DLL_RELEASE
     AND (_isMultiConfig OR CMAKE_BUILD_TYPE))
    # if the generator is multi-config or if CMAKE_BUILD_TYPE is set for
    # single-config generators, set optimized and debug libraries
    set(${basename}_DLL "")
    foreach(_libname IN LISTS ${basename}_DLL_RELEASE)
      list(APPEND ${basename}_DLL optimized "${_libname}")
    endforeach()
    foreach(_libname IN LISTS ${basename}_DLL_DEBUG)
      list(APPEND ${basename}_DLL debug "${_libname}")
    endforeach()
  elseif(${basename}_DLL_RELEASE)
    set(${basename}_DLL ${${basename}_DLL_RELEASE})
  elseif(${basename}_DLL_DEBUG)
    set(${basename}_DLL ${${basename}_DLL_DEBUG})
  else()
    set(${basename}_DLL "${basename}_DLL-NOTFOUND")
  endif()

  mark_as_advanced(${basename}_DLL_RELEASE ${basename}_DLL_DEBUG)
endmacro()
