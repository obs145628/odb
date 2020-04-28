set(SRC
  main.cc
  cli.cc
)
add_executable(odb-cli ${SRC})
target_link_libraries(odb-cli odb_client odb_mess)
