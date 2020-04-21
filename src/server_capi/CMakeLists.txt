set(SRC
  server-app.cc
  vm-api.cc
)
add_library(odb_server_capi ${SRC})
target_link_libraries(odb_server_capi odb_server)
