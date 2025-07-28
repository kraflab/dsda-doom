include(${CMAKE_CURRENT_LIST_DIR}/TestRunnerHelpers.cmake)

if(NOT DEFINED EXPECTED_FINAL_TIME)
  message(FATAL_ERROR "Could not run test: EXPECTED_FINAL_TIME is not set")
endif()

dsda_play_demo(
  "${TEST_LUMP}"
  "${TEST_IWAD}"
  "${TEST_PWAD}"
  ""
)

dsda_parse_levelstat_file(final_time)

if(NOT final_time STREQUAL EXPECTED_FINAL_TIME)
  message(FATAL_ERROR
    "${TEST_LUMP}: Final time do not match:\n"
    "  Expected: ${EXPECTED_FINAL_TIME}\n"
    "  Got:      ${final_time}\n"
  )
endif()
