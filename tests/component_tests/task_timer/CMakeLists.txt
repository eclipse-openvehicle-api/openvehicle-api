# Define project (multiple EXEs)
project(TaskTimerTests VERSION 1.0 LANGUAGES CXX)

# TaskTimer executable
add_executable(ComponentTest_TaskTimer
    task_timer_test.cpp
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(ComponentTest_TaskTimer GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(ComponentTest_TaskTimer Winmm Ws2_32 Rpcrt4.lib)
    else()
        target_link_libraries(ComponentTest_TaskTimer rt ${CMAKE_DL_LIBS})
    endif()
else()
    target_link_libraries(ComponentTest_TaskTimer GTest::GTest Winmm Rpcrt4.lib)
endif()

# Add the tasktimer unittest
add_test(NAME ComponentTest_TaskTimer COMMAND ComponentTest_TaskTimer)

# Execute the test
add_custom_command(TARGET ComponentTest_TaskTimer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_TaskTimer>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_TaskTimer.xml
    VERBATIM
)

# SimulationTaskTimer executable
add_executable(ComponentTest_Simulation_TaskTimer
    task_simulation_timer_test.cpp
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(ComponentTest_Simulation_TaskTimer GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(ComponentTest_Simulation_TaskTimer Winmm Ws2_32 Rpcrt4.lib)
    else()
        target_link_libraries(ComponentTest_Simulation_TaskTimer rt ${CMAKE_DL_LIBS})
    endif()
else()
    target_link_libraries(ComponentTest_Simulation_TaskTimer GTest::GTest Winmm Rpcrt4.lib)
endif()

# Add the tasktimer unittest
add_test(NAME ComponentTest_Simulation_TaskTimer COMMAND ComponentTest_Simulation_TaskTimer)

# Execute the test
add_custom_command(TARGET ComponentTest_Simulation_TaskTimer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_Simulation_TaskTimer>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_Simulation_TaskTimer.xml
    VERBATIM
)


# copy the configuration to output folder 'config'
# and build dependencies
add_dependencies(ComponentTest_TaskTimer dependency_sdv_components)
add_dependencies(ComponentTest_TaskTimer task_timer)
add_dependencies(ComponentTest_Simulation_TaskTimer dependency_sdv_components)
add_dependencies(ComponentTest_Simulation_TaskTimer simulation_task_timer)
file (COPY ${PROJECT_SOURCE_DIR}/test_tt_config.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/config/)
file (COPY ${PROJECT_SOURCE_DIR}/test_simulation_tt_config.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/config/)
