cmake_minimum_required(VERSION 3.10)
project(SecureEdgeChat C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror -g")

include_directories(include src/common)

add_executable(server src/server/server.c src/common/utils.c)
add_executable(client src/client/client.c src/common/utils.c)
