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
if(UNIX)
cmake_minimum_required(VERSION 3.20)

project(UnixTunnelCommunicationTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable: GTest for UDS connect
add_executable(UnitTest_UnixTunnelConnectTests
    unix_tunnel_connect_tests.cpp           
)

target_include_directories(UnitTest_UnixTunnelConnectTests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_UnixTunnelConnectTests
        GTest::GTest
        ${CMAKE_THREAD_LIBS_INIT} 
        stdc++fs 
        ${CMAKE_DL_LIBS} 
        rt
        uds_unix_tunnel
        uds_unix_sockets
    )
else()
    target_link_libraries(UnitTest_UnixTunnelConnectTests
        GTest::GTest 
        Ws2_32 Winmm 
        Rpcrt4.lib
        uds_unix_tunnel
        uds_unix_sockets
    )
endif()

add_test(NAME UnitTest_UnixTunnelConnectTests
         COMMAND UnitTest_UnixTunnelConnectTests)

add_dependencies(UnitTest_UnixTunnelConnectTests dependency_sdv_components)

if ((NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (NOT WIN32))
add_custom_command(TARGET UnitTest_UnixTunnelConnectTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake
            "$<TARGET_FILE:UnitTest_UnixTunnelConnectTests>"
            --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_UnixTunnelConnectTests.xml
    VERBATIM
)
endif()

add_executable(UnitTest_UnixTunnelChannelMgntTests
    unix_tunnel_channel_mgnt_tests.cpp           
)

target_include_directories(UnitTest_UnixTunnelChannelMgntTests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_UnixTunnelChannelMgntTests
        GTest::GTest 
        ${CMAKE_THREAD_LIBS_INIT} 
        stdc++fs 
        ${CMAKE_DL_LIBS} 
        rt
        uds_unix_tunnel
        uds_unix_sockets
    )
else()
    target_link_libraries(UnitTest_UnixTunnelChannelMgntTests
        GTest::GTest 
        Ws2_32 
        Winmm 
        Rpcrt4.lib
        uds_unix_tunnel
        uds_unix_sockets
    )
endif()

add_test(NAME UnitTest_UnixTunnelChannelMgntTests
         COMMAND UnitTest_UnixTunnelChannelMgntTests)

add_dependencies(UnitTest_UnixTunnelChannelMgntTests dependency_sdv_components)

if ((NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (NOT WIN32))
add_custom_command(TARGET UnitTest_UnixTunnelChannelMgntTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake
            "$<TARGET_FILE:UnitTest_UnixTunnelChannelMgntTests>"
            --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_UnixTunnelChannelMgntTests.xml
    VERBATIM
)
endif()

endif()