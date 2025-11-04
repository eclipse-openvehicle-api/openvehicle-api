# Define project
project (UnitTest_CommandLine_Parser VERSION 1.0 LANGUAGES CXX)

if (WIN32)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/bigobj)
else()
    add_compile_options(-Wa,-mbig-obj)
endif()
endif()

# Compile the source code
add_executable(UnitTest_CommandLine_Parser
    "commandline_parser_test.cpp"
    "commandline_parser_test.h"
    "defaultarg_parser_test.cpp"
    "option_parser_test.cpp"
    "suboption_parser_test.cpp"
    "windows_option_parser_test.cpp"
    "main.cpp"
    "option_parser_test_no_assignment_char.cpp"
    "suboption_parser_test_no_assignment_char.cpp"
    "option_parser_test_assignment_next_arg.cpp"
    "suboption_parser_test_assignment_next_arg.cpp"
    "option_argument_selective_group.cpp"
    "suboption_argument_selective_group.cpp"
     "incompatible_arguments.cpp")
target_link_libraries(UnitTest_CommandLine_Parser ${CMAKE_DL_LIBS} GTest::GTest)

# Add the IDL Compiler unittest
add_test(NAME UnitTest_CommandLine_Parser COMMAND UnitTest_CommandLine_Parser)

# Execute the test
add_custom_command(TARGET UnitTest_CommandLine_Parser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_CommandLine_Parser>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_CommandLine_Parser.xml
    VERBATIM
)

# The unit-test project depends on a proper compilation of the compiler before
add_dependencies(UnitTest_CommandLine_Parser dependency_sdv_components)
