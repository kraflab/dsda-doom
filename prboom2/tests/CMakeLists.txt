list(APPEND
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_LIST_DIR}"
)

set(dsda_executable_arg $<TARGET_FILE:dsda-doom>)
if(CMAKE_CROSSCOMPILING AND CMAKE_CROSSCOMPILING_EMULATOR)
  list(PREPEND dsda_executable_arg ${CMAKE_CROSSCOMPILING_EMULATOR})
endif()

include(TestChecks)
check_test_wads()

include(SyncTests)

if(HAVE_DOOM2)
  include(AnalysisTests)
  include(CategoryTests)
endif()

if(HAVE_HERETIC)
  include(HereticTests)
endif()
