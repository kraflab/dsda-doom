# Sets `outvar` to 1 if the target architecture uses big endian, 0 otherwise.
# TestBigEndian was deprecated in 3.20.
function(check_big_endian outvar)
  if(CMAKE_VERSION VERSION_LESS 3.20)
    include(TestBigEndian)
    test_big_endian(_is_big_endian)
  else()
    if(CMAKE_C_BYTE_ORDER STREQUAL "BIG_ENDIAN")
      set(_is_big_endian 1)
    else()
      set(_is_big_endian 0)
    endif()
  endif()
  set("${outvar}"
      "${_is_big_endian}"
      PARENT_SCOPE)
endfunction()
