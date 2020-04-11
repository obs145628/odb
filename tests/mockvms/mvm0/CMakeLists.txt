set(SRC
  src/cpu.cc
  src/parser.cc
  src/vm-api.cc
)
add_library(mock_mvm0 ${SRC})
