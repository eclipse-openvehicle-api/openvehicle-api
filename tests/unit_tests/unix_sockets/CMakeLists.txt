#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#*******************************************************************************

cmake_minimum_required(VERSION 3.20)

project(UnixSocketCommunicationTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable: GTest for UDS connect
add_executable(UnitTest_UnixSocketConnectTests
    uds_connect_tests.cpp           
)

target_include_directories(UnitTest_UnixSocketConnectTests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_UnixSocketConnectTests
        GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs ${CMAKE_DL_LIBS} rt
    )
else()
    target_link_libraries(UnitTest_UnixSocketConnectTests
        GTest::GTest Ws2_32 Winmm Rpcrt4.lib
    )
endif()

add_test(NAME UnitTest_UnixSocketConnectTests
         COMMAND UnitTest_UnixSocketConnectTests)

add_dependencies(UnitTest_UnixSocketConnectTests dependency_sdv_components)

if ((NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (NOT WIN32))
add_custom_command(TARGET UnitTest_UnixSocketConnectTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake
            "$<TARGET_FILE:UnitTest_UnixSocketConnectTests>"
            --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_UnixSocketConnectTests.xml
    VERBATIM
)
endif()