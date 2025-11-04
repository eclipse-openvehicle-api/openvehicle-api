# Enforce CMake version 3.20
cmake_minimum_required (VERSION 3.20)
cmake_policy (VERSION 3.20)

# Define project
project(vapi_framework_examples VERSION 1.0 LANGUAGES CXX)

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)

# Libary symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Re-set target directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/lib>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)

# Default C++ settings
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("Use MSVC compiler...")
    add_compile_options(/W4 /WX /wd4996 /wd4127 /permissive- /Zc:rvalueCast)
    add_compile_definitions(_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING UNICODE _UNICODE)
else()
    # There are some versions of GCC that produce bogus warnings for -Wstringop-overflow (e.g. version 9.4 warns, 11.4 not - changing
    # the compile order without changing the logical behavior, will produce different results).
    # See also: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100477
    # And https://gcc.gnu.org/bugzilla/show_bug.cgi?id=115074
    # Suppress this warning.
    add_compile_options(-Werror -Wall -Wextra -Wshadow -Wpedantic -Wunreachable-code -fno-common -fPIC -ggdb -pthread -Wno-error=stringop-overflow)
    add_link_options(-pthread)
    if (WIN32)
        message("Use g++ compiler under Windows (mingw)...")
        add_compile_definitions(UNICODE _UNICODE main=wmain)
        add_link_options(-municode)
    else()
        message("Use g++ compiler...")
    endif()
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Check for the environment variable for the V-API runtime framework
if (NOT DEFINED SDV_FRAMEWORK_RUNTIME)
    if (NOT DEFINED ENV{SDV_FRAMEWORK_RUNTIME})
        message( FATAL_ERROR "The environment variable SDV_FRAMEWORK_RUNTIME needs to be pointing to the SDV V-API framework location!")
    endif()
    set (SDV_FRAMEWORK_RUNTIME "$ENV{SDV_FRAMEWORK_RUNTIME}")
endif()
message("SDV_FRAMEWORK_RUNTIME set to ${SDV_FRAMEWORK_RUNTIME}")

# Check for the existance of the V-API development tools
if (NOT DEFINED SDV_FRAMEWORK_DEV_TOOLS)
    if (NOT DEFINED ENV{SDV_FRAMEWORK_DEV_TOOLS})
        message( FATAL_ERROR "The environment variable SDV_FRAMEWORK_DEV_TOOLS needs to be pointing to the SDV V-API development tools location!")
    endif()
    set (SDV_FRAMEWORK_DEV_TOOLS "$ENV{SDV_FRAMEWORK_DEV_TOOLS}")
endif()
message("SDV_FRAMEWORK_DEV_TOOLS set to ${SDV_FRAMEWORK_DEV_TOOLS}")
if (NOT DEFINED SDV_FRAMEWORK_DEV_INCLUDE)
    if (NOT DEFINED ENV{SDV_FRAMEWORK_DEV_INCLUDE})
        message( FATAL_ERROR "The environment variable SDV_FRAMEWORK_DEV_INCLUDE needs to be pointing to the SDV V-API development include files location!")
    endif()
    set (SDV_FRAMEWORK_DEV_INCLUDE "$ENV{SDV_FRAMEWORK_DEV_INCLUDE}")
endif()
message("SDV_FRAMEWORK_DEV_INCLUDE set to ${SDV_FRAMEWORK_DEV_INCLUDE}")
if (WIN32)
    set (SDV_IDL_COMPILER "${SDV_FRAMEWORK_DEV_TOOLS}/sdv_idl_compiler.exe")
    set (SDV_DBC_UTIL "${SDV_FRAMEWORK_DEV_TOOLS}/sdv_dbc_util.exe")
    set (SDV_VSS_UTIL "${SDV_FRAMEWORK_DEV_TOOLS}/sdv_vss_util.exe")
    set (SDV_PACKAGER "${SDV_FRAMEWORK_DEV_TOOLS}/sdv_packager.exe")
else()
    set (SDV_IDL_COMPILER "${SDV_FRAMEWORK_DEV_TOOLS}/sdv_idl_compiler")
    set (SDV_DBC_UTIL "${SDV_FRAMEWORK_DEV_TOOLS}/sdv_dbc_util")
    set (SDV_VSS_UTIL "${SDV_FRAMEWORK_DEV_TOOLS}/sdv_vss_util")
    set (SDV_PACKAGER "${SDV_FRAMEWORK_DEV_TOOLS}/sdv_packager")
endif()
if (NOT EXISTS ${SDV_IDL_COMPILER})
    message( FATAL_ERROR "The SDV V-API IDL compiler is missing! \nDoes the SDV_FRAMEWORK_DEV_TOOLS environment variable have the correct path? \nIt is set to ${SDV_IDL_COMPILER}")
endif()
if (NOT EXISTS "${SDV_DBC_UTIL}")
    message( FATAL_ERROR "The SDV V-API DBC utility is missing! \nDoes the SDV_FRAMEWORK_DEV_TOOLS environment variable have the correct path? \nIt is set to ${SDV_IDL_COMPILER}")
endif()
if (NOT EXISTS "${SDV_VSS_UTIL}")
    message( FATAL_ERROR "The SDV V-API VSS utility is missing! \nDoes the SDV_FRAMEWORK_DEV_TOOLS environment variable have the correct path? \nIt is set to ${SDV_IDL_COMPILER}")
endif()
if (NOT EXISTS "${SDV_PACKAGER}")
    message( FATAL_ERROR "The SDV V-API installation packager utility is missing! \nDoes the SDV_FRAMEWORK_DEV_TOOLS environment variable have the correct path? \nIt is set to ${SDV_IDL_COMPILER}")
endif()

add_subdirectory(your_cmake_project)


