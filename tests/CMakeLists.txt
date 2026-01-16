# Define project
project(Tests VERSION 1.0 LANGUAGES CXX)

# Enable testing for all subsequent projects
enable_testing()

# Re-set target directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/tests/lib>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/tests/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/tests/bin>)

# Include export into the include directory path
include_directories(../export)

# Get Google Test from github
include(FetchContent)
# Fetch the Google Test library from GitHub.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY                https://github.com/google/googletest.git
    GIT_TAG                       v1.17.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # For Windows: Prevent overriding the parent project's compiler/linker settings

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

FetchContent_MakeAvailable(googletest)

add_library(GTest::GTest INTERFACE IMPORTED)
add_library(GMock::GMock INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)
target_link_libraries(GMock::GMock INTERFACE gmock_main)

file (COPY ${PROJECT_SOURCE_DIR}/sdv_core_reloc.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/)
file (COPY ${PROJECT_SOURCE_DIR}/sdv_core_reloc.toml DESTINATION ${CMAKE_BINARY_DIR}/examples/bin/)
file (COPY ${PROJECT_SOURCE_DIR}/sdv_core_reloc.toml DESTINATION ${CMAKE_BINARY_DIR}/framework_tests/bin/)

# Copy shell script to the build directory
if(WIN32)
  file (COPY ${PROJECT_SOURCE_DIR}/run_tests_on_windows.ps1 DESTINATION ${CMAKE_BINARY_DIR}/tests/)
else()
  file (COPY ${PROJECT_SOURCE_DIR}/run_tests_on_linux.sh DESTINATION ${CMAKE_BINARY_DIR}/tests/)
endif()

# Add test projects
add_subdirectory(unit_tests/concurrency)
add_subdirectory(unit_tests/flags_type)
add_subdirectory(unit_tests/scheduler)
add_subdirectory(unit_tests/idl_compiler)
add_subdirectory(unit_tests/commandline_parser)
add_subdirectory(unit_tests/dbc_parser)
add_subdirectory(unit_tests/asc_format)
add_subdirectory(unit_tests/basic_types)
add_subdirectory(unit_tests/smart_ifc)
add_subdirectory(unit_tests/toml_parser)
add_subdirectory(unit_tests/module_control)
add_subdirectory(unit_tests/repository)
add_subdirectory(unit_tests/shared_mem)
add_subdirectory(unit_tests/memory_manager)
add_subdirectory(unit_tests/core_loader)
add_subdirectory(unit_tests/named_mutex)
add_subdirectory(unit_tests/trace_fifo)
add_subdirectory(unit_tests/socket_can_com_tests)
add_subdirectory(unit_tests/app_connect)
add_subdirectory(unit_tests/process_control)
add_subdirectory(unit_tests/ipc_com)
add_subdirectory(unit_tests/ipc_connect)
add_subdirectory(unit_tests/sdv_control)
add_subdirectory(unit_tests/sdv_macro_test)
add_subdirectory(unit_tests/install_package_composer)
add_subdirectory(unit_tests/path_match)
add_subdirectory(component_tests/logger)
add_subdirectory(component_tests/data_dispatch_service)
add_subdirectory(component_tests/repository)
add_subdirectory(component_tests/config_install)
add_subdirectory(component_tests/dbc_util)
add_subdirectory(component_tests/vss_util)
add_subdirectory(component_tests/task_timer)
add_subdirectory(component_tests/core_library)
add_subdirectory(component_tests/config)
add_subdirectory(manual_tests/silkit_can_com_tests)
