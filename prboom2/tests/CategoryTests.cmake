include_guard()

function(add_category_test lump_name expected_category)
  add_test(
    NAME dsda.category.${lump_name}
    COMMAND ${CMAKE_COMMAND}
      ${dsda_executable_arg}
      -DTEST_LUMP=${lump_name}.lmp
      -DEXPECTED_CATEGORY=${expected_category}
      -P ${CMAKE_CURRENT_LIST_DIR}/CategoryTestRunner.cmake
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${lump_name}"
  )
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${lump_name}")
  set_tests_properties(dsda.category.${lump_name}
    PROPERTIES
    LABELS category
  )
endfunction()

add_category_test(lv01-005 "UV Speed")
add_category_test(lv01-039 "UV Max")
add_category_test(lv01t040 "UV Tyson")
add_category_test(pa08-020 "Pacifist")
add_category_test(lv08str037 "Stroller")
add_category_test(nm04-036 "NM Speed")
add_category_test(1156ns01 "NM 100S")
add_category_test(lv01o497 "NoMo")
add_category_test(os01-2394 "NoMo 100S")
add_category_test(re02-107 "UV Respawn")
add_category_test(fa04-109 "UV Fast")
add_category_test(d2dtqr "Other")
