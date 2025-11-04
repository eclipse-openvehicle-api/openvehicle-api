# Define project
project(InstallPackageComposerTests VERSION 1.0 LANGUAGES CXX)

# build component #1
add_library(UnitTest_InstallPackageComposer_Component1 SHARED
    dummy_component1.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(UnitTest_InstallPackageComposer_Component1 PUBLIC -fPIC)
    if (WIN32)
        target_link_libraries(UnitTest_InstallPackageComposer_Component1 Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_InstallPackageComposer_Component1 ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_InstallPackageComposer_Component1 Rpcrt4.lib)
endif()

set_target_properties(UnitTest_InstallPackageComposer_Component1 PROPERTIES PREFIX "")
set_target_properties(UnitTest_InstallPackageComposer_Component1 PROPERTIES SUFFIX ".sdv")

# build component #2
add_library(UnitTest_InstallPackageComposer_Component2 SHARED
    dummy_component2.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(UnitTest_InstallPackageComposer_Component2 PUBLIC -fPIC)
    if (WIN32)
        target_link_libraries(UnitTest_InstallPackageComposer_Component2 Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_InstallPackageComposer_Component2 ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_InstallPackageComposer_Component2 Rpcrt4.lib)
endif()

set_target_properties(UnitTest_InstallPackageComposer_Component2 PROPERTIES PREFIX "")
set_target_properties(UnitTest_InstallPackageComposer_Component2 PROPERTIES SUFFIX ".sdv")

# Define target
# TODO EVE
add_executable(UnitTest_InstallPackageComposer
    composer_test_suite.cpp
    composer_test_suite.h
    composer_tests.cpp
    manifest_tests.cpp
    environment_tests.cpp
    package_version_tests.cpp
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(UnitTest_InstallPackageComposer GTest::GTest)
    if (WIN32)
        target_link_libraries(UnitTest_InstallPackageComposer Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(UnitTest_InstallPackageComposer ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(UnitTest_InstallPackageComposer GTest::GTest Rpcrt4.lib)
endif()

# Add the test
add_test(NAME UnitTest_InstallPackageComposer COMMAND UnitTest_InstallPackageComposer WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Execute the test
add_custom_command(TARGET UnitTest_InstallPackageComposer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env TEST_EXECUTION_MODE=CMake "$<TARGET_FILE:UnitTest_InstallPackageComposer>" --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTest_InstallPackageComposer.xml
    VERBATIM
)

# TODO EVE
add_dependencies(UnitTest_InstallPackageComposer_Component1 dependency_sdv_components)
add_dependencies(UnitTest_InstallPackageComposer_Component2 dependency_sdv_components)
add_dependencies(UnitTest_InstallPackageComposer UnitTest_InstallPackageComposer_Component1)
add_dependencies(UnitTest_InstallPackageComposer UnitTest_InstallPackageComposer_Component2)
add_dependencies(UnitTest_InstallPackageComposer dependency_sdv_components)
