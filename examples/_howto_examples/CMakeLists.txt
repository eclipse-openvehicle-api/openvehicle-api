project(HowToExamples)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)

# Libary symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Include link to export directory of SDV V-API development include files location
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

add_library(example_vehicle_device SHARED source/example_vehicle_device.cpp)
set_target_properties(example_vehicle_device PROPERTIES PREFIX "")
set_target_properties(example_vehicle_device PROPERTIES SUFFIX ".sdv")

add_library(example_basic_service SHARED source/example_basic_service.cpp)
set_target_properties(example_basic_service PROPERTIES PREFIX "")
set_target_properties(example_basic_service PROPERTIES SUFFIX ".sdv")

add_library(example_access_to_other_components SHARED source/example_access_to_other_components.cpp)
set_target_properties(example_access_to_other_components PROPERTIES PREFIX "")
set_target_properties(example_access_to_other_components PROPERTIES SUFFIX ".sdv")

add_library(example_general_component SHARED source/example_general_component.cpp)
set_target_properties(example_general_component PROPERTIES PREFIX "")
set_target_properties(example_general_component PROPERTIES SUFFIX ".sdv")

add_library(example_general_component_with_initialization SHARED source/example_general_component_with_initialization.cpp)
set_target_properties(example_general_component_with_initialization PROPERTIES PREFIX "")
set_target_properties(example_general_component_with_initialization PROPERTIES SUFFIX ".sdv")

add_library(example_dispatch_service SHARED source/example_dispatch_service.cpp)
set_target_properties(example_dispatch_service PROPERTIES PREFIX "")
set_target_properties(example_dispatch_service PROPERTIES SUFFIX ".sdv")

add_library(example_complex_service SHARED source/example_complex_service.cpp)
set_target_properties(example_complex_service PROPERTIES PREFIX "")
set_target_properties(example_complex_service PROPERTIES SUFFIX ".sdv")

#---------------------------------------
#       Verify all HowTo examples
#---------------------------------------
add_executable(example_run_all_components "source/run_all_examples.cpp")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(example_run_all_components Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(example_run_all_components ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})       
    endif()
else()
    target_link_libraries(example_run_all_components Rpcrt4.lib)   
endif()



# Copy the config files for the application example
file (COPY ${PROJECT_SOURCE_DIR}/source/docu_dispatch.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/source/docu_examples.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
file (COPY ${PROJECT_SOURCE_DIR}/source/docu_app_examples.toml DESTINATION ${CMAKE_BINARY_DIR}/bin/config)

#---------------------------------
#       Application examples
#---------------------------------
add_executable(example_application source/example_application.cpp)


if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        target_link_libraries(example_application Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(example_application ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})        
    endif()
else()
    target_link_libraries(example_application Rpcrt4.lib)    
endif()


add_dependencies(example_run_all_components  example_vehicle_device example_complex_service 
                                    example_dispatch_service example_general_component 
                                    example_access_to_other_components example_basic_service 
                                    example_general_component_with_initialization)


add_dependencies(example_application  example_general_component example_access_to_other_components)
