# Define project
project(PathMatchTestTests VERSION 1.0 LANGUAGES CXX)

# Define target
add_executable(UnitTest_PathMatchTest
    path_match_test_suite.cpp
    path_match_test_suite.h
    wildcard_match.cpp
     "regex_match.cpp")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_PathMatchTest GTest::GTest)
    if (WIN32)
        target_link_libraries(UnitTest_PathMatchTest Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_PathMatchTest ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_PathMatchTest GTest::GTest Rpcrt4.lib)
endif()

# Add the test
add_test(NAME UnitTest_PathMatchTest COMMAND UnitTest_PathMatchTest WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET UnitTest_PathMatchTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_PathMatchTest>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_PathMatchTest.xml
    VERBATIM
)
