#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#
# Contributors:
#   Denisa Ros - initial API and implementation
#*******************************************************************************
if(WIN32)
cmake_minimum_required(VERSION 3.20)

project(WinTunnelCommunicationTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable: GTest for UDS connect
add_executable(UnitTest_WinTunnelConnectTests
    win_tunnel_connect_tests.cpp           
)

target_include_directories(UnitTest_WinTunnelConnectTests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/export
        ${PROJECT_SOURCE_DIR}/export/support
        ${PROJECT_SOURCE_DIR}/export/interfaces
        ${PROJECT_SOURCE_DIR}/sdv_services
)

target_link_libraries(UnitTest_WinTunnelConnectTests
    PRIVATE
        GTest::GTest
        Ws2_32
        Winmm
        Rpcrt4
        uds_win_tunnel
        uds_win_sockets
)

add_test(NAME UnitTest_WinTunnelConnectTests
         COMMAND UnitTest_WinTunnelConnectTests)

add_dependencies(UnitTest_WinTunnelConnectTests dependency_sdv_components)

if ((NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (NOT WIN32))
add_custom_command(TARGET UnitTest_WinTunnelConnectTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake
            "$<TARGET_FILE:UnitTest_WinTunnelConnectTests>"
            --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_WinTunnelConnectTests.xml
    VERBATIM
)
endif()

add_executable(UnitTest_WinTunnelChannelMgntTests
    win_tunnel_channel_mgnt_tests.cpp           
)

target_include_directories(UnitTest_WinTunnelChannelMgntTests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/export
        ${PROJECT_SOURCE_DIR}/export/support
        ${PROJECT_SOURCE_DIR}/export/interfaces
        ${PROJECT_SOURCE_DIR}/sdv_services
)

target_link_libraries(UnitTest_WinTunnelChannelMgntTests
    PRIVATE
        GTest::GTest
        Ws2_32
        Winmm
        Rpcrt4
        uds_win_tunnel
        uds_win_sockets
)

add_test(NAME UnitTest_WinTunnelChannelMgntTests
         COMMAND UnitTest_WinTunnelChannelMgntTests)

add_dependencies(UnitTest_WinTunnelChannelMgntTests dependency_sdv_components)

if ((NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (NOT WIN32))
add_custom_command(TARGET UnitTest_WinTunnelChannelMgntTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake
            "$<TARGET_FILE:UnitTest_WinTunnelChannelMgntTests>"
            --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_WinTunnelChannelMgntTests.xml
    VERBATIM
)
endif()

# Add negative/edge case tests
add_executable(UnitTest_WinTunnelNegativeEdgeTests
    win_tunnel_negative_edge_tests.cpp
)

target_include_directories(UnitTest_WinTunnelNegativeEdgeTests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/export
        ${PROJECT_SOURCE_DIR}/export/support
        ${PROJECT_SOURCE_DIR}/export/interfaces
        ${PROJECT_SOURCE_DIR}/sdv_services
)

target_link_libraries(UnitTest_WinTunnelNegativeEdgeTests
    PRIVATE
        GTest::GTest
        Ws2_32
        Winmm
        Rpcrt4
        uds_win_tunnel
        uds_win_sockets
)

add_test(NAME UnitTest_WinTunnelNegativeEdgeTests
         COMMAND UnitTest_WinTunnelNegativeEdgeTests)

add_dependencies(UnitTest_WinTunnelNegativeEdgeTests dependency_sdv_components)

if ((NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (NOT WIN32))
add_custom_command(TARGET UnitTest_WinTunnelNegativeEdgeTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake
            "$<TARGET_FILE:UnitTest_WinTunnelNegativeEdgeTests>"
            --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_WinTunnelNegativeEdgeTests.xml
    VERBATIM
)
endif()

endif()
