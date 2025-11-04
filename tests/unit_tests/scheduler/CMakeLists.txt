# Define project
project(SchedulerTests VERSION 1.0 LANGUAGES CXX)

# Define target
add_executable(UnitTest_Scheduler scheduler_test.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_Scheduler GTest::GTest)
    if (WIN32)
        target_link_libraries(UnitTest_Scheduler Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_Scheduler ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_Scheduler GTest::GTest Rpcrt4.lib)
endif()

# Add the test
add_test(NAME UnitTest_Scheduler COMMAND UnitTest_Scheduler WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET UnitTest_Scheduler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_Scheduler>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_Scheduler.xml
    VERBATIM
)
