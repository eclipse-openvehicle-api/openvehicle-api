# Only valid for Linux
if(UNIX)

    project(UnitTestCanComSocket)

    # Include directories
    include_directories(../../export)
    include_directories(${CMAKE_SOURCE_DIR}/../sdv_services/can_communication_socket_can)

    # Add the executable
    add_executable(UnitTest_CANComSocket
        src/can_com_test_socket.cpp
        ${CMAKE_SOURCE_DIR}/sdv_services/can_communication_socket_can/can_com_sockets.cpp
    )

    # Link libraries
    target_link_libraries(UnitTest_CANComSocket ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} rt GTest::GTest)

    # Add test
    add_test(NAME UnitTest_CANComSocket COMMAND UnitTest_CANComSocket)

    # Execute the test
    add_custom_command(TARGET UnitTest_CANComSocket POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_CANComSocket>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unitTest_CANComSocket.xml
        VERBATIM
    )

    # Copy the config files
    file (COPY ${PROJECT_SOURCE_DIR}/config/test_can_socket.toml DESTINATION ${CMAKE_BINARY_DIR}/tests/bin/config)

    # Build dependencies
    add_dependencies(UnitTest_CANComSocket dependency_sdv_components)
    add_dependencies(UnitTest_CANComSocket can_com_sockets)

endif()
