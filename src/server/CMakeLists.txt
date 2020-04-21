set(SRC
  cli-client-handler.cc
  client-handler.cc
  db-client-impl-vmside.cc
  debugger.cc
  multi-client-handler.cc
  server-app.cc
)
add_library(odb_server ${SRC})
target_link_libraries(odb_server odb_mess)
