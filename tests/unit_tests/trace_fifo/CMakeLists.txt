# Define project
project(UnitTest_TraceFifo VERSION 1.0 LANGUAGES CXX)

# Add executable
add_executable(UnitTest_TraceFifo
    "main.cpp"
    "fifo_test.cpp"
    )

# Link target
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_TraceFifo GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_TraceFifo Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_TraceFifo ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_TraceFifo GTest::GTest Rpcrt4.lib)
endif()

# Add test
add_test(NAME UnitTest_TraceFifo COMMAND UnitTest_TraceFifo)

# Execute test
add_custom_command(TARGET UnitTest_TraceFifo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_TraceFifo>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_TraceFifo.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_TraceFifo core_services)
