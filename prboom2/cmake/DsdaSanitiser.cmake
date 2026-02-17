include_guard()

include(CheckCCompilerFlag)
include(CMakeDependentOption)

if(MSVC)
  check_c_compiler_flag(/fsanitize=address HAVE_ASAN)
  cmake_dependent_option(DSDA_ENABLE_ASAN
    "Enable address sanitiser"
    OFF "HAVE_ASAN" OFF
  )

  if(DSDA_ENABLE_ASAN)
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    add_compile_options(/fsanitize=address /fsanitize-address-use-after-return)
  endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  set(sanitiser_names address undefined thread)
  set(sanitiser_ids ASAN UBSAN TSAN)
  foreach(sanitiser_name sanitiser_id IN ZIP_LISTS sanitiser_names sanitiser_ids)
    list(APPEND CMAKE_REQUIRED_LINK_OPTIONS -fsanitize=${sanitiser_name})
    check_c_compiler_flag(-fsanitize=${sanitiser_name} HAVE_${sanitiser_id})
    cmake_dependent_option(DSDA_ENABLE_${sanitiser_id}
      "Enable ${sanitiser_name} sanitiser"
      OFF "HAVE_${sanitiser_id}" OFF
    )
    list(POP_BACK CMAKE_REQUIRED_LINK_OPTIONS)
  endforeach()

  if(DSDA_ENABLE_ASAN AND DSDA_ENABLE_TSAN)
    message(FATAL_ERROR
      "Invalid sanitizer combination:\n"
      "  DSDA_ENABLE_ASAN: ${DSDA_ENABLE_ASAN}\n"
      "  DSDA_ENABLE_UBSAN: ${DSDA_ENABLE_UBSAN}\n"
      "  DSDA_ENABLE_TSAN: ${DSDA_ENABLE_TSAN}\n"
    )
  endif()

  add_compile_options(
    -fno-omit-frame-pointer
    $<$<BOOL:${DSDA_ENABLE_ASAN}>:-fsanitize=address>
    $<$<BOOL:${DSDA_ENABLE_UBSAN}>:-fsanitize=undefined>
    $<$<BOOL:${DSDA_ENABLE_TSAN}>:-fsanitize=thread>
  )
  add_link_options(
    $<$<BOOL:${DSDA_ENABLE_ASAN}>:-fsanitize=address>
    $<$<BOOL:${DSDA_ENABLE_UBSAN}>:-fsanitize=undefined>
    $<$<BOOL:${DSDA_ENABLE_TSAN}>:-fsanitize=thread>
  )
endif()
