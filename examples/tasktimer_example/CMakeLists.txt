project(TaskTimerApp)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

set(CMAKE_CXX_STANDARD 17)

# Define the executable
add_executable(tasktimer_example
    src/tasktimer_example.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(tasktimer_example Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(tasktimer_example ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(tasktimer_example Rpcrt4.lib)
endif()

# Include link to export directory of SDV V-API development include files location
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

target_link_libraries(tasktimer_example
    ${CMAKE_DL_LIBS}
)

add_test(NAME tasktimer_example COMMAND tasktimer_example)

# Define the executable
add_executable(simulation_tasktimer_example
    src/simulation_tasktimer_example.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(simulation_tasktimer_example Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(simulation_tasktimer_example ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
    endif()
else()
    target_link_libraries(simulation_tasktimer_example Rpcrt4.lib)
endif()

# Include link to export directory of SDV V-API development include files location
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})


target_link_libraries(simulation_tasktimer_example
    ${CMAKE_DL_LIBS}
)

add_test(NAME simulation_tasktimer_example COMMAND simulation_tasktimer_example)

# Copy the config files
file (COPY ${PROJECT_SOURCE_DIR}/config/test_config_task_timer.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/config/test_config_simulation_task_timer.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)


