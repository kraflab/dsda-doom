include_guard()

function(add_sync_test lump_name expected_time iwad pwad)
  add_test(
    NAME dsda.sync.${lump_name}
    COMMAND ${CMAKE_COMMAND}
      "-DDSDA_DOOM_EXECUTABLE=${dsda_executable_arg}"
      -DTEST_LUMP=${lump_name}.lmp
      -DEXPECTED_FINAL_TIME=${expected_time}
      -DTEST_IWAD=${iwad}
      -DTEST_PWAD=${pwad}
      -P ${CMAKE_CURRENT_LIST_DIR}/SyncTestRunner.cmake
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${lump_name}"
  )
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${lump_name}")
  set_tests_properties(dsda.sync.${lump_name}
    PROPERTIES
    LABELS sync
  )
endfunction()

if(HAVE_DOOM2)
  add_sync_test(30uv1755 17:55 DOOM2.WAD "")
  add_sync_test(cm20k222 2:22 DOOM2.WAD "")
  add_sync_test(cm20t222 2:22 DOOM2.WAD "")

  if(HAVE_RUSH)
    add_sync_test(ru12-2114 21:14 DOOM2.WAD rush.wad)
  endif()

  if(HAVE_VALIANT)
    add_sync_test(vae1-513 5:13 DOOM2.WAD Valiant.wad)
  endif()
endif()

if(HAVE_HERETIC)
  add_sync_test(h1m-5240 52:40 HERETIC.WAD "")
  add_sync_test(h2ma6702 67:02 HERETIC.WAD "")
  add_sync_test(h3ma6248 62:48 HERETIC.WAD "")
  add_sync_test(h4sp1055 10:55 HERETIC.WAD "")
  add_sync_test(h5sp1257 12:57 HERETIC.WAD "")
endif()

if(HAVE_HEXEN)
  add_sync_test(me1c4537 45:37 HEXEN.WAD "")
endif()
