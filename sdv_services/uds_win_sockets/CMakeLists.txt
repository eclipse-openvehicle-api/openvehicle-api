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
project(uds_win_sockets VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(uds_win_sockets SHARED
    "channel_mgnt.h"
    "channel_mgnt.cpp"
    "connection.h"
    "connection.cpp")

target_link_libraries(uds_win_sockets ${CMAKE_THREAD_LIBS_INIT} Ws2_32.lib)

target_link_options(uds_win_sockets PRIVATE)
target_include_directories(uds_win_sockets PRIVATE ./include/)

set_target_properties(uds_win_sockets PROPERTIES PREFIX "")
set_target_properties(uds_win_sockets PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(uds_win_sockets CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} uds_win_sockets PARENT_SCOPE)

endif()
