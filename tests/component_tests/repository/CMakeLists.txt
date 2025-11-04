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
    OUTPUT ${PROJECT_SOURCE_DIR}/generated/test_component.h
    DEPENDS sdv_idl_compiler
    MAIN_DEPENDENCY test_component.idl
    COMMENT "Build test_component.idl"
    COMMAND ${SDVIDL} "${PROJECT_SOURCE_DIR}/test_component.idl" "-I${PROJECT_SOURCE_DIR}/../../../export" "-O${PROJECT_SOURCE_DIR}/generated"
    VERBATIM
    )

set_source_files_properties(test_component_ps.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)
set_source_files_properties(test_component.cpp OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/generated/test_component.h)


# Add the dynamic library
add_library(ComponentTest_Repository_ps SHARED
    "test_component_ps.cpp"
    )

# Set extension to .sdv
set_target_properties(ComponentTest_Repository_ps PROPERTIES PREFIX "")
set_target_properties(ComponentTest_Repository_ps PROPERTIES SUFFIX ".sdv")

# build module example for the test
add_library(ComponentTest_Repository_test_module SHARED
    test_component.cpp
    )

target_link_libraries(ComponentTest_Repository_test_module ${CMAKE_DL_LIBS} GTest::GTest)

set_target_properties(ComponentTest_Repository_test_module PROPERTIES PREFIX "")
set_target_properties(ComponentTest_Repository_test_module PROPERTIES SUFFIX ".sdv")

# Execute the installation helper utility to create an installation manifest for the core files.
add_custom_target(ComponentTest_Repository_install_manifest
# TODO EVE
#    COMMAND "$<TARGET_FILE:sdv_packager>" -O. --instance2005 -NComponentTest_Repository "$<TARGET_FILE:ComponentTest_Repository_test_module>" "$<TARGET_FILE:ComponentTest_Repository_ps>" "-I$<TARGET_FILE_DIR:ComponentTest_Repository_test_module>" --settings --create_configtest.toml --exclude_config_class"TestObject_CreateChain" --exclude_config_class"TestObject_CreateChainLock" --exclude_config_class"TestObject_CreateChainLockThread" --exclude_config_class"TestObject_CreateDuringShutdown" --exclude_config_class"TestObject_IObjectControlFail"
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL ComponentTest_Repository --instance2005 "$<TARGET_FILE:ComponentTest_Repository_test_module>" "$<TARGET_FILE:ComponentTest_Repository_ps>" "-I$<TARGET_FILE_DIR:ComponentTest_Repository_test_module>" --overwrite --user_config
    DEPENDS ComponentTest_Repository_test_module ComponentTest_Repository_ps
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Define target
add_executable(ComponentTest_Repository
    "repository_test.cpp"
    "isolation_test.cpp"
    )

target_link_libraries(ComponentTest_Repository ${CMAKE_DL_LIBS} GTest::GTest)

# Add the test
add_test(NAME ComponentTest_Repository COMMAND ComponentTest_Repository WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET ComponentTest_Repository POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_Repository>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_Repository.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_Repository_test_module install_manifest)
add_dependencies(ComponentTest_Repository_test_module dependency_sdv_components)
add_dependencies(ComponentTest_Repository dependency_sdv_components)
add_dependencies(ComponentTest_Repository_ps dependency_sdv_components)
add_dependencies(ComponentTest_Repository ComponentTest_Repository_test_module)
add_dependencies(ComponentTest_Repository ComponentTest_Repository_ps)
add_dependencies(ComponentTest_Repository ComponentTest_Repository_install_manifest)