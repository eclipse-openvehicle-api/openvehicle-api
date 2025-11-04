# Character Reader executable
add_executable(UnitTest_TOMLParser
    "character_reader_tests.cpp"
    "lexer_tests.cpp"
    "parser_tests.cpp"
    "main.cpp"
    "generate_toml_tests.cpp")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_TOMLParser GTest::GTest ${CMAKE_THREAD_LIBS_INIT})
    if (WIN32)
        target_link_libraries(UnitTest_TOMLParser Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_TOMLParser ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_TOMLParser GTest::GTest Rpcrt4.lib)
endif()

# Add the Reader unittest
add_test(NAME UnitTest_TOMLParser COMMAND UnitTest_TOMLParser)

# Execute the test
add_custom_command(TARGET UnitTest_TOMLParser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_TOMLParser>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_TOMLParser.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_TOMLParser core_services)



