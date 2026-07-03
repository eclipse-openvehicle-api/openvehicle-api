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

# Build sources once and expose them in two forms:
# - uds_unix_sockets: runtime-loadable .sdv module for ModuleControl
# - uds_unix_sockets_static: static link target for unit tests / static consumers
set(UDS_UNIX_SOCKETS_SOURCES
    channel_mgnt.cpp
    connection.cpp
    watchdog.cpp
)

add_library(uds_unix_sockets SHARED
    ${UDS_UNIX_SOCKETS_SOURCES}
)

add_library(uds_unix_sockets_static STATIC
    ${UDS_UNIX_SOCKETS_SOURCES}
)

target_include_directories(uds_unix_sockets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ./include/
)

target_include_directories(uds_unix_sockets_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ./include/
)

target_link_libraries(uds_unix_sockets
    PUBLIC
        rt
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(uds_unix_sockets_static
    PUBLIC
        rt
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
)

set_target_properties(uds_unix_sockets PROPERTIES PREFIX "")
set_target_properties(uds_unix_sockets PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(uds_unix_sockets CompileCoreIDL)
add_dependencies(uds_unix_sockets_static CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} uds_unix_sockets PARENT_SCOPE)

endif()