set(TEST_SRC
  test_main.cc
  test_range_map.cc
)
set(TEST_NAME utest_utils.bin)
add_executable(${TEST_NAME} EXCLUDE_FROM_ALL ${TEST_SRC}) 
add_dependencies(build-tests ${TEST_NAME})
