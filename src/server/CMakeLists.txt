set(SRC
  cli-client-handler.cc
  client-handler.cc
  data-client-handler.cc
  db-client-impl-vmside.cc
  debugger.cc
  multi-client-handler.cc
  server-app.cc
  tcp-data-server.cc
)
add_library(odb_server ${SRC})
target_link_libraries(odb_server odb_mess pthread)
