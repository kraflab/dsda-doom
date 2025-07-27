include_guard()

function(dsda_check_test_environment)
  if(NOT DEFINED DSDA_DOOM_EXECUTABLE)
    message(FATAL_ERROR "DSDA_DOOM_EXECUTABLE is not set")
  endif()

  list(GET DSDA_DOOM_EXECUTABLE -1 real_executable)

  if(NOT EXISTS "${real_executable}")
    message(FATAL_ERROR "${real_executable} does not exist")
  endif()

  if(NOT DEFINED TEST_LUMP)
    message(FATAL_ERROR "TEST_LUMP is not set")
  endif()
endfunction()

function(dsda_fail_if_file_not_exist filename)
  if(NOT EXISTS ${filename})
    message(FATAL_ERROR "File \"${filename}\" does not exist.")
  endif()
endfunction()

function(dsda_parse_analysis_file result_prefix)
  dsda_fail_if_file_not_exist(analysis.txt)

  file(READ
    analysis.txt
    analysis_data
  )

  set(expected_keys
    skill
    nomonsters
    respawn
    fast
    pacifist
    stroller
    reality
    almost_reality
    reborn
    100k
    100s
    missed_monsters
    missed_secrets
    weapon_collector
    tyson_weapons
    turbo
    solo_net
    coop_spawns
    category
    signature
  )

  foreach(key IN LISTS expected_keys)
    string(REGEX MATCH "${key} ([^\n]+)" _ ${analysis_data})
    set("${result_prefix}_${key}"
      ${CMAKE_MATCH_1}
      PARENT_SCOPE
    )
  endforeach()
endfunction()

function(dsda_parse_levelstat_file result_time)
  dsda_fail_if_file_not_exist(levelstat.txt)

  file(STRINGS
    "levelstat.txt"
    levelstat_data
  )

  list(POP_BACK levelstat_data last_level_data)
  string(REGEX MATCH "\\(([0-9:]+)\\)" _ "${last_level_data}")
  if(CMAKE_MATCH_1)
    set(final_time ${CMAKE_MATCH_1})
  else()
    set(final_time "00:00")
  endif()

  set(${result_time}
    ${final_time}
    PARENT_SCOPE
  )
endfunction()

function(dsda_play_demo lump iwad pwad extra_args)
  set(cmd_pwad_args "")
  if(pwad)
    set(cmd_pwad_args
      "-file"
      "${CMAKE_CURRENT_LIST_DIR}/WAD/${pwad}"
    )
  endif()

  execute_process(
    COMMAND
      ${DSDA_DOOM_EXECUTABLE}
      -iwad
      "${CMAKE_CURRENT_LIST_DIR}/WAD/${iwad}"
      ${cmd_pwad_args}
      -fastdemo
      "${CMAKE_CURRENT_LIST_DIR}/lmps/${lump}"
      -nosound
      -nomusic
      -nodraw
      -levelstat
      -analysis
      ${extra_args}
    COMMAND_ECHO STDERR
  )
endfunction()

dsda_check_test_environment()
