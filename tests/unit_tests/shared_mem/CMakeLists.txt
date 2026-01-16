# Define project (multiple EXEs)
project(CommunicationTests VERSION 1.0 LANGUAGES CXX)

# Surrogate repeater executable
add_executable(UnitTest_SharedMemTests_App_Repeater
 "app_repeater.cpp"
 "pattern_gen.cpp"
 )
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_SharedMemTests_App_Repeater ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_SharedMemTests_App_Repeater Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_SharedMemTests_App_Repeater ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_SharedMemTests_App_Repeater Ws2_32 Winmm Rpcrt4.lib)
endif()

# Surrogate repeater executable
add_executable(UnitTest_SharedMemTests_App_Connect
 "app_connect.cpp"
 )
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_SharedMemTests_App_Connect ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_SharedMemTests_App_Connect Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_SharedMemTests_App_Connect ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_SharedMemTests_App_Connect Ws2_32 Winmm Rpcrt4.lib)
endif()

# In process mem test
add_executable(UnitTest_InprocMemTests
    "in_process_mem_buffer_tests.cpp"
    "pattern_gen.cpp"
    "pattern_gen.h"
    "main.cpp"
    )
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_InprocMemTests GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_InprocMemTests Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_InprocMemTests ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_InprocMemTests GTest::GTest Rpcrt4.lib)
endif()

# Add the communication unittest
add_test(NAME UnitTest_InprocMemTests COMMAND UnitTest_InprocMemTests)

# Execute the test
add_custom_command(TARGET UnitTest_InprocMemTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_InprocMemTests>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_InprocMemTests.xml
    VERBATIM
)

# Shared mem buffer test
add_executable(UnitTest_SharedMemBufferTests
    "pattern_gen.cpp"
    "pattern_gen.h"
    "main.cpp"
    "shared_mem_buffer_tests.cpp"
    )
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_SharedMemBufferTests GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_SharedMemBufferTests Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_SharedMemBufferTests ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_SharedMemBufferTests GTest::GTest Rpcrt4.lib)
endif()

# Add the communication unittest
add_test(NAME UnitTest_SharedMemBufferTests COMMAND UnitTest_SharedMemBufferTests)

# Execute the test
add_custom_command(TARGET UnitTest_SharedMemBufferTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_SharedMemBufferTests>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_SharedMemBufferTests.xml
    VERBATIM
)

# Shared mem connection test
add_executable(UnitTest_SharedMemConnectTests
    "main.cpp"
    "shared_mem_connect.cpp"
    )
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_SharedMemConnectTests GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_SharedMemConnectTests Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_SharedMemConnectTests ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_SharedMemConnectTests GTest::GTest Rpcrt4.lib)
endif()

# Add the communication unittest
add_test(NAME UnitTest_SharedMemConnectTests COMMAND UnitTest_SharedMemConnectTests)

# Execute the test
add_custom_command(TARGET UnitTest_SharedMemConnectTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_SharedMemConnectTests>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_SharedMemConnectTests.xml
    VERBATIM
)


# Shared mem large data test
add_executable(UnitTest_SharedMemLargeDataTests
    "main.cpp"
    "shared_mem_large_data_tests.cpp"
    )
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_SharedMemLargeDataTests GTest::GTest ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(UnitTest_SharedMemLargeDataTests Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_SharedMemLargeDataTests ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_SharedMemLargeDataTests GTest::GTest Rpcrt4.lib)
endif()

# Add the communication unittest
add_test(NAME UnitTest_SharedMemLargeDataTests COMMAND UnitTest_SharedMemLargeDataTests)

# Execute the test
add_custom_command(TARGET UnitTest_SharedMemLargeDataTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_SharedMemLargeDataTests>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_SharedMemLargeDataTests.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_SharedMemTests_App_Repeater core_services)
add_dependencies(UnitTest_SharedMemTests_App_Connect core_services)
add_dependencies(UnitTest_InprocMemTests core_services)
add_dependencies(UnitTest_SharedMemBufferTests UnitTest_SharedMemTests_App_Repeater)
add_dependencies(UnitTest_SharedMemConnectTests UnitTest_SharedMemTests_App_Connect)
add_dependencies(UnitTest_SharedMemLargeDataTests UnitTest_SharedMemTests_App_Connect)
