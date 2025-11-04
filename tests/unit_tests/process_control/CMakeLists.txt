# Define project (multiple EXEs)
project(ProcessControlUnitTests VERSION 1.0 LANGUAGES CXX)

# Surrogate repeater executable
add_executable(UnitTest_ProcessControlApp
 "process_control_app.cpp"
  "process_control_ifc.h")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_ProcessControlApp ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_ProcessControlApp Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_ProcessControlApp ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_ProcessControlApp Ws2_32 Winmm Rpcrt4.lib)
endif()

# Communication tests executable
add_executable(UnitTest_ProcessControl

 "main.cpp"
 "process_control_tests.cpp" "process_control_ifc.h")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_ProcessControl GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_ProcessControl Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_ProcessControl ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_ProcessControl GTest::GTest Rpcrt4.lib)
endif()

# Add the communication unittest
add_test(NAME UnitTest_ProcessControl COMMAND UnitTest_ProcessControl)

# Execute the test
add_custom_command(TARGET UnitTest_ProcessControl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_ProcessControl>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_ProcessControl.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_ProcessControlApp dependency_sdv_components)
add_dependencies(UnitTest_ProcessControl UnitTest_ProcessControlApp)
add_dependencies(UnitTest_ProcessControl dependency_sdv_components)
