cmake_minimum_required(VERSION 2.6)
project(verteiler)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DOPENSSL)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../socket-cpp/Socket)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../socket-cpp/cmake-build-debug/lib/Debug)

add_library(verteiler_lieferant lieferant/lieferant.cpp lieferant/lieferant.hpp allg/standardlogg.hpp allg/string_add.hpp allg/ThreadFIFO.hpp)
add_library(verteiler_kunde kunde/kunde.cpp kunde/kunde.hpp allg/standardlogg.hpp allg/ThreadFIFO.hpp)
target_link_libraries(verteiler_lieferant socket pthread ssl crypto)
target_link_libraries(verteiler_kunde socket pthread ssl crypto)

add_executable(Testclient test/Testclient.cpp)
target_link_libraries(Testclient verteiler_kunde)

add_executable(Testserver test/Testserver.cpp)
target_link_libraries(Testserver verteiler_lieferant)

#if(EXISTS key.pem)
#else()
#    add_custom_command(TARGET verteiler_lieferant
#            POST_BUILD
#            COMMAND /bin/sh ../certs.sh
#            )
#endif()
