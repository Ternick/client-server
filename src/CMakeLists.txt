cmake_minimum_required(VERSION 3.10)
project(ClientServer)
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads REQUIRED)

add_library(server_lib server/server.cpp server/server.h)
target_include_directories(server_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/server)

add_library(packet_lib 
    server/packet/packet.cpp 
    server/packet/packet.h 
    server/packet/crc32.cpp 
    server/packet/crc32.h)
target_include_directories(packet_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/server/packet)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE pthread packet_lib)

add_executable(server serv.cpp)
target_link_libraries(server PRIVATE server_lib packet_lib pthread)