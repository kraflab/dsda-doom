include_guard()

function(add_analysis_test lump_name analysis_key expected_result)
  cmake_parse_arguments(PARSE_ARGV 0 arg "WITH_ANALYSIS_PWAD" "" "")
  add_test(
    NAME dsda.analysis.${analysis_key}.${lump_name}
    COMMAND ${CMAKE_COMMAND}
      ${dsda_executable_arg}
      -DTEST_LUMP=${lump_name}.lmp
      -DWITH_ANALYSIS_PWAD=${arg_WITH_ANALYSIS_PWAD}
      -DANALYSIS_KEY=${analysis_key}
      -DEXPECTED_RESULT=${expected_result}
      -P ${CMAKE_CURRENT_LIST_DIR}/AnalysisTestRunner.cmake
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${lump_name}_${analysis_key}"
  )
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${lump_name}_${analysis_key}")
  set_tests_properties(dsda.analysis.${analysis_key}.${lump_name}
    PROPERTIES
    LABELS analysis
  )
endfunction()

add_analysis_test(nm04-036 skill 5)
add_analysis_test(lv01-005 skill 4)

add_analysis_test(lv01o497 nomonsters 1)
add_analysis_test(lv01-005 nomonsters 0)

add_analysis_test(re02-107 respawn 1)
add_analysis_test(nm04-036 respawn 0)

add_analysis_test(fa04-109 fast 1)
add_analysis_test(nm04-036 fast 0)

add_analysis_test(lv01-039 100k 1)
add_analysis_test(lv01-005 100k 0)

add_analysis_test(1156ns01 100s 1)
add_analysis_test(lv01-005 100s 0)

add_analysis_test(lve3-2654 missed_monsters 0)
add_analysis_test(lve3-2654 missed_secrets 1)

add_analysis_test(barrel_chain pacifist 0 WITH_ANALYSIS_PWAD)
add_analysis_test(barrel_assist pacifist 0 WITH_ANALYSIS_PWAD)
add_analysis_test(keen pacifist 0 WITH_ANALYSIS_PWAD)
add_analysis_test(romero pacifist 0 WITH_ANALYSIS_PWAD)
add_analysis_test(splash pacifist 0 WITH_ANALYSIS_PWAD)
add_analysis_test(telefrag pacifist 1 WITH_ANALYSIS_PWAD)
add_analysis_test(voodoo pacifist 1 WITH_ANALYSIS_PWAD)

add_analysis_test(lv08str037 stroller 1)
add_analysis_test(pa08-020 stroller 0)

add_analysis_test(damage reality 0 WITH_ANALYSIS_PWAD)
add_analysis_test(nukage reality 0 WITH_ANALYSIS_PWAD)
add_analysis_test(crusher reality 0 WITH_ANALYSIS_PWAD)
add_analysis_test(reality reality 1 WITH_ANALYSIS_PWAD)
add_analysis_test(damage almost_reality 0 WITH_ANALYSIS_PWAD)
add_analysis_test(nukage almost_reality 1 WITH_ANALYSIS_PWAD)
add_analysis_test(crusher almost_reality 0 WITH_ANALYSIS_PWAD)
add_analysis_test(reality almost_reality 0 WITH_ANALYSIS_PWAD)

add_analysis_test(lv01t040 tyson_weapons 1)
add_analysis_test(lv01-039 tyson_weapons 0)

add_analysis_test(d2dtqr turbo 1)
add_analysis_test(lv01-039 turbo 0)

add_analysis_test(cl01-022 weapon_collector 1)
add_analysis_test(lv01t040 weapon_collector 0)
