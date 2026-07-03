#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#*******************************************************************************

if(UNIX)
# Define project
project(uds_unix_tunnel VERSION 1.0 LANGUAGES CXX)

set(UDS_UNIX_TUNNEL_SOURCES
    channel_mgnt.cpp
    connection.cpp
    watchdog.cpp
)

# Define targets
add_library(uds_unix_tunnel STATIC
    ${UDS_UNIX_TUNNEL_SOURCES}
    )

add_library(uds_unix_tunnel_module SHARED
    ${UDS_UNIX_TUNNEL_SOURCES}
    )

target_link_libraries(uds_unix_tunnel
    PRIVATE
        uds_unix_sockets_static
        rt
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(uds_unix_tunnel_module
    PRIVATE
        uds_unix_sockets_static
        rt
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(uds_unix_tunnel
    PRIVATE
        ./include/
        ../uds_unix_sockets/
)

target_include_directories(uds_unix_tunnel_module
    PRIVATE
        ./include/
        ../uds_unix_sockets/
)
set_target_properties(uds_unix_tunnel_module PROPERTIES PREFIX "")
set_target_properties(uds_unix_tunnel_module PROPERTIES SUFFIX ".sdv")
set_target_properties(uds_unix_tunnel_module PROPERTIES OUTPUT_NAME "uds_unix_tunnel")

# Build dependencies
add_dependencies(uds_unix_tunnel CompileCoreIDL)
add_dependencies(uds_unix_tunnel_module CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} uds_unix_tunnel_module PARENT_SCOPE)

endif()