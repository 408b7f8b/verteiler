cmake_minimum_required(VERSION 2.6)
project(verteiler)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DOPENSSL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../socket-cpp/Socket)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../socket-cpp/cmake-build-debug/lib/Debug)

add_library(verteiler_sender Sender/Sender.cpp Sender/Sender.hpp)
add_library(verteiler_receiver Receiver/Receiver.cpp Receiver/Receiver.hpp)
target_link_libraries(verteiler_sender socket pthread ssl crypto)
target_link_libraries(verteiler_receiver socket pthread ssl crypto)

add_executable(Testclient test/Testclient.cpp)
target_link_libraries(Testclient verteiler_receiver)

add_executable(Testserver test/Testserver.cpp)
target_link_libraries(Testserver verteiler_sender)