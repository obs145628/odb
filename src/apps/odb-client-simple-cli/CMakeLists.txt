set(SRC
  main.cc
)
add_executable(odb-client-simple-cli EXCLUDE_FROM_ALL ${SRC})
target_link_libraries(odb-client-simple-cli odb_client odb_mess)
