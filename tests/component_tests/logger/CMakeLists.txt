project(LoggerTests)

set(CMAKE_CXX_STANDARD 17)

# Define target
add_library(ComponentTest_Logger_Module SHARED "logger_test_service.h" "logger_test_service.cpp")
target_link_options(ComponentTest_Logger_Module PRIVATE)
target_include_directories(ComponentTest_Logger_Module PRIVATE ./include)
if (WIN32)
    target_link_libraries(ComponentTest_Logger_Module Ws2_32 Winmm)
else()
    target_link_libraries(ComponentTest_Logger_Module)
endif()
set_target_properties(ComponentTest_Logger_Module PROPERTIES PREFIX "")
set_target_properties(ComponentTest_Logger_Module PROPERTIES SUFFIX ".sdv")

# Logger
add_executable(ComponentTest_Logger
    "logger_test.cpp"
)

target_include_directories(ComponentTest_Logger PRIVATE
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(ComponentTest_Logger
    GTest::GTest
    #pthread
    #stdc++fs
    ${CMAKE_DL_LIBS}
)

add_test(NAME ComponentTest_Logger COMMAND ComponentTest_Logger)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(ComponentTest_Logger Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(ComponentTest_Logger rt)
    endif()
else()
    target_link_libraries(ComponentTest_Logger Rpcrt4.lib)
endif()

add_custom_command(TARGET ComponentTest_Logger POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_Logger>" ${GTEST_ARGUMENTS}ComponentTest_Logger.xml
    VERBATIM
)

file (COPY ${PROJECT_SOURCE_DIR}/config/test_logger_userconfig.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/config/)

add_dependencies(ComponentTest_Logger_Module dependency_sdv_components)
add_dependencies(ComponentTest_Logger ComponentTest_Logger_Module)
