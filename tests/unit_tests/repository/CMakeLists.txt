# Define project
project(RepositoryTests VERSION 1.0 LANGUAGES CXX)

# Find the IDL compiler
# REMARKS The compiler can only be found after it has build. Having both the compiler and the unittest project build, causes an
# error during the scanning phase of CMake.
#find_program(SDVIDL NAMES "sdv_idl_compiler" PATHS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/" NO_CACHE)
set (SDVIDL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../../bin/sdv_idl_compiler")
message("Use IDL compiler: ${SDVIDL}")

# Compile the IDL
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/IComponent.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY IComponent.idl
    COMMENT "Build IComponent.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/IComponent.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )

# build module example for the test
add_library(UnitTest_Repository_test_module SHARED test_component.cpp generated/IComponent.h)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(UnitTest_Repository_test_module PUBLIC -fPIC)
    target_link_libraries(UnitTest_Repository_test_module GTest::GTest)
    if (WIN32)
        target_link_libraries(UnitTest_Repository_test_module Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_Repository_test_module ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_Repository_test_module GTest::GTest Rpcrt4.lib)
endif()

set_target_properties(UnitTest_Repository_test_module PROPERTIES PREFIX "")
set_target_properties(UnitTest_Repository_test_module PROPERTIES SUFFIX ".sdv")

# Define target
add_executable(UnitTest_Repository repository_test.cpp generated/IComponent.h "mock.h" "main.cpp")


if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(UnitTest_Repository PUBLIC -fPIC)
    target_link_libraries(UnitTest_Repository GTest::GTest)
    if (WIN32)
        target_link_libraries(UnitTest_Repository Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_Repository ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_Repository GTest::GTest Rpcrt4.lib)
endif()

# Add the test
add_test(NAME UnitTest_Repository COMMAND UnitTest_Repository WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET UnitTest_Repository POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_Repository>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_Repository.xml
    VERBATIM
)

# Build dependencies
add_dependencies(UnitTest_Repository_test_module dependency_sdv_components)
add_dependencies(UnitTest_Repository dependency_sdv_components)
add_dependencies(UnitTest_Repository UnitTest_Repository_test_module)