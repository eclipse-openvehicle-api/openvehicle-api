# Define project
project (DataDispatchServiceTests VERSION 1.0 LANGUAGES CXX)

# Data maneger executable
add_executable(ComponentTest_DataDispatchService data_dispatch_service_test.cpp "transaction_test.cpp" "main.cpp" "trigger_test.cpp")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries(ComponentTest_DataDispatchService ${CMAKE_THREAD_LIBS_INIT} GTest::GTest)
    if (WIN32)
        target_link_libraries(ComponentTest_DataDispatchService Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(ComponentTest_DataDispatchService ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(ComponentTest_DataDispatchService GTest::GTest Rpcrt4.lib)
endif()

# Add the Data Dispatch Service unittest
add_test(NAME ComponentTest_DataDispatchService COMMAND ComponentTest_DataDispatchService WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET ComponentTest_DataDispatchService POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_DataDispatchService>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_DataDispatchService.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_DataDispatchService dependency_sdv_components)
add_dependencies(ComponentTest_DataDispatchService data_dispatch_service)
add_dependencies(ComponentTest_DataDispatchService task_timer)
file (COPY ${PROJECT_SOURCE_DIR}/test_dds_config.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/config/)
