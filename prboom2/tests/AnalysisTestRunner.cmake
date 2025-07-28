include(${CMAKE_CURRENT_LIST_DIR}/TestRunnerHelpers.cmake)

if(NOT DEFINED EXPECTED_RESULT)
  message(FATAL_ERROR "Could not run test: EXPECTED_RESULT is not set")
endif()

if(NOT DEFINED ANALYSIS_KEY)
  message(FATAL_ERROR "Could not run test: ANALYSIS_KEY is not set")
endif()

if(WITH_ANALYSIS_PWAD)
  set(pwad analysis_test.wad)
endif()

dsda_play_demo(
  "${TEST_LUMP}"
  "DOOM2.WAD"
  "${pwad}"
  ""
)

dsda_parse_analysis_file(analysis)

if(NOT analysis_${ANALYSIS_KEY} STREQUAL EXPECTED_RESULT)
  message(FATAL_ERROR
    "${TEST_LUMP}: Analysis result for ${ANALYSIS_KEY} do not match:\n"
    "  Expected: ${EXPECTED_RESULT}\n"
    "  Got:      ${analysis_${ANALYSIS_KEY}}\n"
  )
endif()
