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

# Define project
project(uds_win_tunnel VERSION 1.0 LANGUAGES CXX)

set(UDS_WIN_TUNNEL_SOURCES
    channel_mgnt.cpp
    connection.cpp
    watchdog.cpp
)

# Define targets
add_library(uds_win_tunnel STATIC
    ${UDS_WIN_TUNNEL_SOURCES}
)

add_library(uds_win_tunnel_module SHARED
    ${UDS_WIN_TUNNEL_SOURCES}
)

target_link_libraries(uds_win_tunnel
    PRIVATE
        uds_win_sockets
        Ws2_32.lib
        ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(uds_win_tunnel_module
    PRIVATE
        uds_win_sockets
        Ws2_32.lib
        ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(uds_win_tunnel
    PRIVATE
        ./include/
        ../uds_win_sockets/
)

target_include_directories(uds_win_tunnel_module
    PRIVATE
        ./include/
        ../uds_win_sockets/
)

set_target_properties(uds_win_tunnel_module PROPERTIES
    PREFIX ""
    SUFFIX ".sdv"
    OUTPUT_NAME "uds_win_tunnel"
    ARCHIVE_OUTPUT_NAME "uds_win_tunnel_module"
)

# Build dependencies
add_dependencies(uds_win_tunnel CompileCoreIDL)
add_dependencies(uds_win_tunnel_module CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} uds_win_tunnel_module PARENT_SCOPE)

endif()
