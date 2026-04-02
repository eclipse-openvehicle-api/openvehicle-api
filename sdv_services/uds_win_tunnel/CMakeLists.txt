#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#*******************************************************************************

# Define project
project(uds_win_tunnel VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(uds_win_tunnel STATIC
    channel_mgnt.cpp
    connection.cpp
)

target_link_libraries(uds_win_tunnel
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

set_target_properties(uds_win_tunnel PROPERTIES PREFIX "")
set_target_properties(uds_win_tunnel PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(uds_win_tunnel CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} uds_win_tunnel PARENT_SCOPE)
