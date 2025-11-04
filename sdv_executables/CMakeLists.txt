# Include cross-compilation toolchain file
include(../cross-compile-tools.cmake)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Define project
project(SDVServices VERSION 1.0 LANGUAGES CXX)

# Include export into the include directory path
include_directories(../export)

# Add executable projects
add_subdirectory(sdv_idl_compiler)
add_subdirectory(sdv_dbc_util)
add_subdirectory(sdv_vss_util)
add_subdirectory(sdv_control)
add_subdirectory(sdv_core)
add_subdirectory(sdv_iso)
add_subdirectory(sdv_packager)
add_subdirectory(sdv_local_shutdown)
add_subdirectory(sdv_trace_mon)

# Appending all executables to the service list
set(SDV_Executable_List ${SDV_Executable_List} PARENT_SCOPE)
