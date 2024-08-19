# Copyright (c) 2024, Kiner Shah
# This source code is distributed under BSD-3 Clause License
# details of which can be found in LICENSE file.

cmake_minimum_required(VERSION 3.15)
project(pinger VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)

set(PINGER_SOURCES
    utils.cpp
    WindowsRawSocket.cpp
    PosixDatagramSocket.cpp
    Socket.cpp
    ipv4_header.cpp
    icmp_header.cpp
    Pinger.cpp)

find_package(Threads REQUIRED)
if (NOT Threads_FOUND)
    message(FATAL_ERROR "Threads not found")
endif()

add_library(pinger STATIC ${PINGER_SOURCES})

add_executable(testpinger test.cpp)
target_link_libraries(testpinger LINK_PUBLIC pinger Threads::Threads)
if (MSVC OR WIN32)
    target_link_libraries(testpinger LINK_PUBLIC Ws2_32)
endif()