# Only build on MSVC/Windows or 64-bit Linux (not ARM)
if((WIN32 AND NOT MSVC) OR (UNIX AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64"))

    project(ManualTestCanComSilKit)

    FetchContent_MakeAvailable(SilKit)

    set(SilKit_DIR "${silkit_SOURCE_DIR}/SilKit/lib/cmake/SilKit")

    find_package(SilKit REQUIRED CONFIG)

    if(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(-Wno-shadow)
    endif()

    add_executable(ManualTest_CANComSilKit
        src/can_com_manual_test_silkit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../sdv_services/can_communication_silkit/can_com_silkit.cpp
    )

    add_executable(can_writer
        src/can_writer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../sdv_services/can_communication_silkit/can_com_silkit.cpp
    )

    add_executable(can_reader
        src/can_reader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../sdv_services/can_communication_silkit/can_com_silkit.cpp
    )

    # Copy the config files
    if(UNIX)
        file(COPY ${PROJECT_SOURCE_DIR}/run_silkit_util.sh DESTINATION ${CMAKE_BINARY_DIR}/tests/manual_tests/)
    elseif(WIN32)
        file(COPY ${PROJECT_SOURCE_DIR}/run_silkit_util.bat DESTINATION ${CMAKE_BINARY_DIR}/tests/manual_tests/)
    endif()

    target_link_libraries(ManualTest_CANComSilKit ${CMAKE_DL_LIBS} GTest::GTest SilKit::SilKit)
    target_link_libraries(can_writer ${CMAKE_DL_LIBS} GTest::GTest SilKit::SilKit )
    target_link_libraries(can_reader ${CMAKE_DL_LIBS} GTest::GTest SilKit::SilKit)

    add_test(NAME ManualTest_CANComSilKit COMMAND ManualTest_CANComSilKit)

    # add_custom_command(TARGET ManualTest_CANComSilKit POST_BUILD
    # COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ManualTest_CANComSilKit>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ManualTest_CANComSilKit.xml
    # VERBATIM
    # )

    # Build dependencies
    add_dependencies(ManualTest_CANComSilKit dependency_sdv_components can_com_silkit)
    add_dependencies(can_reader dependency_sdv_components can_com_silkit)
    add_dependencies(can_writer dependency_sdv_components can_com_silkit)

endif() # Only build on MSVC/Windows or 64-bit Linux (not ARM)

