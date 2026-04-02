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
project(uds_unix_sockets VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(uds_unix_sockets STATIC
    channel_mgnt.cpp
    connection.cpp
    )

target_link_libraries(uds_unix_sockets rt ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(uds_unix_sockets PRIVATE ./include/)
set_target_properties(uds_unix_sockets PROPERTIES PREFIX "")
set_target_properties(uds_unix_sockets PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(uds_unix_sockets CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} uds_unix_sockets PARENT_SCOPE)

endif()