# Enforce CMake version 3.20 or newer needed for path function
cmake_minimum_required (VERSION 3.20)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Define project
project(can_dl_door VERSION 1.0 LANGUAGES CXX)

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)

# Library symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Set the SDV_FRAMEWORK_DEV_INCLUDE if not defined yet
if (NOT DEFINED SDV_FRAMEWORK_DEV_INCLUDE)
    if (NOT DEFINED ENV{SDV_FRAMEWORK_DEV_INCLUDE})
        message( FATAL_ERROR "The environment variable SDV_FRAMEWORK_DEV_INCLUDE needs to be pointing to the SDV V-API development include files location!")
    endif()
    set (SDV_FRAMEWORK_DEV_INCLUDE "$ENV{SDV_FRAMEWORK_DEV_INCLUDE}")
endif()

# Include link to export directory of SDV V-API development include files location
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

# Set platform specific compile flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX /wd4996 /wd4100 /permissive- /Zc:rvalueCast)
else()
    add_compile_options(-Werror -Wall -Wextra -Wshadow -Wpedantic -Wunreachable-code -fno-common)
endif()

# Add the dynamic library
add_library(can_dl_door SHARED
    datalink.cpp
    datalink.h)

# Set extension to .sdv
set_target_properties(can_dl_door PROPERTIES PREFIX "")
set_target_properties(can_dl_door PROPERTIES SUFFIX ".sdv")
