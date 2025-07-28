include(${CMAKE_CURRENT_LIST_DIR}/TestRunnerHelpers.cmake)

if(NOT DEFINED EXPECTED_CATEGORY)
  message(FATAL_ERROR "Could not run test: EXPECTED_CATEGORY is not set")
endif()

dsda_play_demo(
  "${TEST_LUMP}"
  "DOOM2.WAD"
  ""
  ""
)

dsda_parse_analysis_file(analysis)

if(NOT analysis_category STREQUAL EXPECTED_CATEGORY)
  message(FATAL_ERROR
    "${TEST_LUMP}: Categories do not match:\n"
    "  Expected: ${EXPECTED_CATEGORY}\n"
    "  Got:      ${analysis_category}\n"
  )
endif()
