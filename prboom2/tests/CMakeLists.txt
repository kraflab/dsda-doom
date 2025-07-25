list(APPEND
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_LIST_DIR}"
)

set(dsda_executable_arg -DDSDA_DOOM_EXECUTABLE=$<TARGET_FILE:dsda-doom>)

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
