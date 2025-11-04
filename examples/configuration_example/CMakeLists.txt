project(ConfigurationTestApp)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)


add_executable(configuration_example  src/configuration_example.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(configuration_example Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(configuration_example ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(configuration_example Rpcrt4.lib)
endif()

# Include link to export directory of SDV V-API development include files location
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

# Copy the config files
file (COPY ${PROJECT_SOURCE_DIR}/config/test_configuration_example.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)

target_link_libraries(configuration_example
    ${CMAKE_DL_LIBS}
)

add_test(NAME configuration_example COMMAND configuration_example )

# Build dependencies
add_dependencies(configuration_example configuration_component_example)
