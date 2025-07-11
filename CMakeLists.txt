cmake_minimum_required(VERSION 3.10)
project(SecureEdgeChat C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror -g")

# All include paths declared together
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/src/crypto
)

# Server and Client executables
add_executable(server
    src/server/server.c
    src/common/utils.c
)

add_executable(client
    src/client/client.c
    src/common/utils.c
)

# Link crypto library (OpenSSL)
add_library(aes_crypto src/crypto/aes_crypto.c)

target_link_libraries(server aes_crypto ssl crypto pthread uuid)
target_link_libraries(client aes_crypto ssl crypto)

