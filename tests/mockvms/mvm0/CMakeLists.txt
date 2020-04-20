add_definitions(-DMVM0_EXS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples/")
add_definitions(-DBUILD_DIR="${CMAKE_BINARY_DIR}/")

set(SRC
  src/cpu.cc
  src/parser.cc
  src/vm-api.cc
)
add_library(mock_mvm0 ${SRC})

set(APP_SRC
  src/main.cc
)
add_executable(mock-mvm0-app ${APP_SRC})
target_link_libraries(mock-mvm0-app mock_mvm0 odb_server)

set(TEST_SRC
  tests/test_main.cc
  tests/test_server_db.cc
  tests/test_server_db_fact.cc
  tests/test_simplecli_add.cc
  tests/test_simplecli_sum.cc
)
set(TEST_NAME utest_mockvms_mvm0.bin)
add_executable(${TEST_NAME} EXCLUDE_FROM_ALL ${TEST_SRC})
target_link_libraries(${TEST_NAME} mock_mvm0 odb_server)
add_dependencies(${TEST_NAME} mock_mvm0)
add_dependencies(build-tests ${TEST_NAME})
