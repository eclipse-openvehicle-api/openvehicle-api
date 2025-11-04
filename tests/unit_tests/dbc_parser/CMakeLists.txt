# Define project
project (UnitTest_DBC_Parser VERSION 1.0 LANGUAGES CXX)

# Compile the source code
add_executable(UnitTest_DBC_Parser
    "dbc_parser_test.cpp"
    "dbc_parser_test.h"
    "main.cpp")
target_link_libraries(UnitTest_DBC_Parser ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_DBC_Parser COMMAND UnitTest_DBC_Parser)

# Execute the test
add_custom_command(TARGET UnitTest_DBC_Parser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_DBC_Parser>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_DBC_Parser.xml
    VERBATIM
)

# The unit-test project depends on a proper compilation of the compiler before
add_dependencies(UnitTest_DBC_Parser dependency_sdv_components)
