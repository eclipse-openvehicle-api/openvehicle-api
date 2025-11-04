# Define project
project (ModuleControlTests VERSION 1.0 LANGUAGES CXX)

# Get CRC++ from github
include(FetchContent)
FetchContent_Declare(
  CRCpp
  GIT_REPOSITORY                https://github.com/d-bahr/CRCpp
  GIT_TAG                       release-1.2.0.0
)
FetchContent_MakeAvailable(CRCpp)

# Add include directories
include_directories(../export ${CRCpp_SOURCE_DIR})

# build module example for the test
add_library(ComponentTest_ConfigInstall_Module SHARED
    "test_component.cpp"
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(ComponentTest_ConfigInstall_Module PUBLIC -fPIC)
    target_link_libraries(ComponentTest_ConfigInstall_Module GTest::GTest)
    if (WIN32)
        target_link_libraries(ComponentTest_ConfigInstall_Module Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(ComponentTest_ConfigInstall_Module ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(ComponentTest_ConfigInstall_Module GTest::GTest Rpcrt4.lib)
endif()

set_target_properties(ComponentTest_ConfigInstall_Module PROPERTIES PREFIX "")
set_target_properties(ComponentTest_ConfigInstall_Module PROPERTIES SUFFIX ".sdv")

# Execute the sdv_packager utility to install the component in instance #2006.
add_custom_target(ComponentTest_ConfigInstall_install_manifest
# TODO EVE
#    COMMAND "$<TARGET_FILE:sdv_packager>" -O. --instance2006 -NComponentTest_ConfigInstall "$<TARGET_FILE:ComponentTest_ConfigInstall_Module>" "-I$<TARGET_FILE_DIR:ComponentTest_ConfigInstall_Module>" --settings --create_configtest.toml
    COMMAND "$<TARGET_FILE:sdv_packager>" DIRECT_INSTALL ComponentTest_ConfigInstall --instance2006 "$<TARGET_FILE:ComponentTest_ConfigInstall_Module>" "-I$<TARGET_FILE_DIR:ComponentTest_ConfigInstall_Module>" --overwrite --user_config
    DEPENDS ComponentTest_ConfigInstall_Module
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Module control test executable
add_executable(ComponentTest_ConfigInstall
    "main.cpp"
    "module_install_test.cpp"
    "manifest_helper_util_test.cpp"
    )

target_link_libraries(ComponentTest_ConfigInstall ${CMAKE_DL_LIBS} GTest::GTest)

# Add the Data Dispatch Service unittest
add_test(NAME ComponentTest_ConfigInstall COMMAND ComponentTest_ConfigInstall WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET ComponentTest_ConfigInstall POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:ComponentTest_ConfigInstall>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ComponentTest_ConfigInstall.xml
    VERBATIM
)

# Build dependencies
add_dependencies(ComponentTest_ConfigInstall_Module sdv_packager)
add_dependencies(ComponentTest_ConfigInstall_Module dependency_sdv_components)
add_dependencies(ComponentTest_ConfigInstall dependency_sdv_components)
add_dependencies(ComponentTest_ConfigInstall ComponentTest_ConfigInstall_Module)
add_dependencies(ComponentTest_ConfigInstall ComponentTest_ConfigInstall_install_manifest)

