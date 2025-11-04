# Define project
project (UnitTest_NamedMutex VERSION 1.0 LANGUAGES CXX)

# Compile the source code
add_executable(UnitTest_NamedMutex
    "named_mutex_test.cpp")
target_link_libraries(UnitTest_NamedMutex ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_NamedMutex COMMAND UnitTest_NamedMutex)

# Execute the test
add_custom_command(TARGET UnitTest_NamedMutex POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_NamedMutex>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_NamedMutex.xml
    VERBATIM
)
