#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#*******************************************************************************
if(WIN32)
cmake_minimum_required(VERSION 3.20)

project(WinSocketCommunicationTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable: GTest for UDS connect
add_executable(UnitTest_WinSocketConnectTests
    win_connect_tests.cpp           
)

target_include_directories(UnitTest_WinSocketConnectTests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/export
        ${PROJECT_SOURCE_DIR}/export/support
        ${PROJECT_SOURCE_DIR}/export/interfaces
        ${PROJECT_SOURCE_DIR}/sdv_services
)

    target_link_libraries(UnitTest_WinSocketConnectTests
        PRIVATE
            GTest::GTest
            # Sockets & timing
            Ws2_32
            Winmm
            Rpcrt4
            uds_win_sockets
    )

add_test(NAME UnitTest_WinSocketConnectTests
         COMMAND UnitTest_WinSocketConnectTests)

add_dependencies(UnitTest_WinSocketConnectTests dependency_sdv_components)

if ((NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (NOT WIN32))
add_custom_command(TARGET UnitTest_WinSocketConnectTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake
            "$<TARGET_FILE:UnitTest_WinSocketConnectTests>"
            --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_WinSocketConnectTests.xml
    VERBATIM
)
endif()

endif()