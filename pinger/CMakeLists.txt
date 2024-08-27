# Copyright (c) 2024, Kiner Shah
# This source code is distributed under BSD-3 Clause License
# details of which can be found in LICENSE file.

cmake_minimum_required(VERSION 3.15)
project(pinger VERSION 0.2)

# Configurable options
option(PINGER_BUILD_SAMPLES "Build samples that demonstrate how to use libpinger" ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

# Set sources for pinger library
set(PINGER_SOURCES
    src/utils.cpp
    src/WindowsRawSocket.cpp
    src/PosixDatagramSocket.cpp
    src/Socket.cpp
    src/ipv4_header.cpp
    src/icmp_header.cpp
    src/PingerImpl.cpp)

# Find threads package
find_package(Threads REQUIRED)
if (NOT Threads_FOUND)
    message(FATAL_ERROR "Threads not found")
endif()

# Create pinger library
add_library(pinger STATIC ${PINGER_SOURCES})

# Set header and library paths required by pinger library
target_include_directories(pinger PUBLIC include)
target_link_libraries(pinger LINK_PRIVATE Threads::Threads)
if (MSVC OR WIN32)
    target_link_libraries(pinger LINK_PRIVATE Ws2_32)
endif()

# Create install rules
install(TARGETS pinger DESTINATION lib)
install(FILES include/Pinger.hpp DESTINATION include)
install(FILES ../LICENSE DESTINATION include/..)

# Create sample application
if (PINGER_BUILD_SAMPLES)
    add_executable(testpinger test.cpp)
    target_link_libraries(testpinger LINK_PUBLIC pinger Threads::Threads)
endif()