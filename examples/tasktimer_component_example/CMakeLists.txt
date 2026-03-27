#*******************************************************************************
# Copyright (c) 2025-2026 ZF Friedrichshafen AG
#
# This program and the accompanying materials are made available under the 
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0 
#*******************************************************************************

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Define project
project(tasktimer_component_example VERSION 1.0 LANGUAGES CXX)

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)

# Libary symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Include link to export directory of SDV V-API development include files location
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

# Add the dynamic library
add_library(tasktimer_component_example SHARED src/tasktimer_comp.cpp)

# Set extension to .sdv
set_target_properties(tasktimer_component_example PROPERTIES PREFIX "")
set_target_properties(tasktimer_component_example PROPERTIES SUFFIX ".sdv")
