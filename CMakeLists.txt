# Enforce CMake version 3.20
cmake_minimum_required (VERSION 3.20)
cmake_policy (VERSION 3.20)

# Check for the environment variable for the V-API runtime framework
if (NOT DEFINED SDV_FRAMEWORK_RUNTIME)
    if (NOT DEFINED ENV{SDV_FRAMEWORK_RUNTIME})
        message( FATAL_ERROR "The environment variable SDV_FRAMEWORK_RUNTIME needs to be pointing to the SDV V-API framework location!")
    endif()
    set (SDV_FRAMEWORK_RUNTIME "$ENV{SDV_FRAMEWORK_RUNTIME}")
endif()
message("SDV_FRAMEWORK_RUNTIME set to ${SDV_FRAMEWORK_RUNTIME}")

# Check for the environment variable for the V-API component installation directory
if (NOT DEFINED SDV_COMPONENT_INSTALL)
    if (NOT DEFINED ENV{SDV_COMPONENT_INSTALL})
        message( FATAL_ERROR "The environment variable SDV_COMPONENT_INSTALL needs to be pointing to the SDV V-API component installation location!")
    endif()
    set (SDV_COMPONENT_INSTALL "$ENV{SDV_COMPONENT_INSTALL}")
endif()
message("SDV_COMPONENT_INSTALL set to ${SDV_COMPONENT_INSTALL}")

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

# Define project
project(vapi_framework-test VERSION 1.0 LANGUAGES CXX)

# This project provides targets to be used in a CMake project depending on components of the framework when
# building everything from scratch
#    sdv_idl_compiler - The target for the IDL-compiler executable used to generate header files from .idl files
#                       located in the sdv_idl_compiler subdirectory
#    dependency_sdv_services - A target that includes all SDV-services fitting for the current build configuration
#                              located in the sdv_services subdirectory
#    dependency_sdv_executables - A target that includes all SDV-executables fitting for the current build configuration
#                                 located in the sdv_executables subdirectory
#    dependency_sdv_components - A target that includes all SDV-services and SDV-executables fitting for the current build
#                                configuration.

# Exclude (so far) known not supported GCC compiler versions
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 9.3)
        message( FATAL_ERROR "GNU gcc compiler version 9.3 cannot be used:
        Bitmask-Evaluation (std::stod function) in minor (9.3) version is different and fails unit-test: TOMLLexerTest.Datatype_Float (minus nan type).

        [from lexer_tests.cc]
        EXPECT_EQ(0x7FF0000000000000ull, *reinterpret_cast<uint64_t*>(&specialFloat6.ContentFloatingpoint) & 0xFFF0000000000000);

        [link/docs]
        sf6 = -nan # valid, actual encoding is implementation-specific

        https://toml.io/en/v1.0.0#float")
    endif()
endif()

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)

# Libary symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Set target directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/lib>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)

if (CMAKE_STATIC_CODE_ANALYSIS)
    message ("Static Code Analysis has been enabled...")
endif()

# Default C++ settings
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("Use MSVC compiler...")
    add_compile_options(/W4 /WX /wd4996 /wd4127 /permissive- /Zc:rvalueCast)

    if (CMAKE_ENABLE_PREFAST)
        message("Microsoft PREfast static code analysis enabled...")
        # Add analysis option, preventing building. Exclude any directory in the CMake binary target dir (here the 3rd party libraries are stored).
        message ("Ignore ${CMAKE_BINARY_DIR}/_deps")
        add_compile_options(/analyze /analyze:external- /external:anglebrackets /external:W0 /external:I ${CMAKE_BINARY_DIR}/_deps /external:templates-)
    endif()

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

# This variable is to be expanded for every service target that is to be built in the current configuration.
# It is used as a dependency list for the `sdv_services` target.
set(SDV_Service_List "")
set(SDV_Executable_List "")
set(SDV_Analysis_List "")

# Include projects
add_subdirectory(sdv_services)
add_subdirectory(sdv_executables)

message("The following SDV services, executables and core analysis are part of the current configuration:")
foreach(service ${SDV_Service_List})
    message("   " ${service})
endforeach()
foreach(executable ${SDV_Executable_List})
    message("   " ${executable})
endforeach()
foreach(analysis ${SDV_Analysis_List})
    message("   " ${analysis})
endforeach()

# This target is intended to be used by projects that depend on the framework and the existence of one or more SDV services.
add_custom_target(dependency_sdv_services
    DEPENDS ${SDV_Service_List}
)
add_custom_target(dependency_sdv_executables
    DEPENDS ${SDV_Executable_List}
)
add_custom_target(dependency_sdv_components
    DEPENDS ${SDV_Service_List}
    DEPENDS ${SDV_Executable_List}
)

# Include cppcheck functionality
include (cppcheck.cmake)

add_custom_target(sdv_framework
    DEPENDS dependency_sdv_components
)

# Blocking cross compilation and static code analysis of tests
if (NOT CMAKE_STATIC_CODE_ANALYSIS AND (NOT DEFINED VAPI_CROSS_COMPILE OR NOT VAPI_CROSS_COMPILE STREQUAL "arm"))
    if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
        add_subdirectory(tests)
    endif()
endif()

