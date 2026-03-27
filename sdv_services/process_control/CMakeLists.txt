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
#   Erik Verhoeven - initial API and implementation
#*******************************************************************************

# Define project
project(process_control VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(process_control SHARED
    "process_control.h"
    "process_control.cpp")
target_link_options(process_control PRIVATE)
target_include_directories(process_control PRIVATE ./include/)
target_link_libraries(process_control ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(process_control PROPERTIES PREFIX "")
set_target_properties(process_control PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(process_control CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} process_control PARENT_SCOPE)
