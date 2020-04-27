set(SRC
  db-client.cc
  request.cc
  simple-cli-client.cc
  tcp-transfer.cc
)
add_library(odb_mess ${SRC})
