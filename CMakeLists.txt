cmake_minimum_required(VERSION 3.0)
project(odb)

# Create log file for odb server
add_definitions(-DODB_SERVER_DB_LOG)

include_directories(SYSTEM "./extern/Catch2/single_include/")
include_directories("./include/")

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -Werror -O0 -g3")

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra -Werror -O0 -g3")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(tests)

add_subdirectory(src/client)
add_subdirectory(src/mess)
add_subdirectory(src/server)
add_subdirectory(src/server_capi)
add_subdirectory(src/utils)

add_subdirectory(src/apps/odb-client-simple-cli)
