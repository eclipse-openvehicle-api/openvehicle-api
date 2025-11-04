# Define project
project (sdv_dbc_util VERSION 1.0 LANGUAGES CXX)

# Add include directories
include_directories(../export)

# Define the executable
add_executable(sdv_dbc_util
    main.cpp
    can_dl.h
    can_dl.cpp
    fmu.h
    fmu.cpp
    cmake_generator.cpp
    cmake_generator.h
    codegen_base.cpp 
    "fmu_templates.h")

if (WIN32)
target_link_libraries(sdv_dbc_util ${CMAKE_DL_LIBS} Rpcrt4.lib)
else()
target_link_libraries(sdv_dbc_util ${CMAKE_DL_LIBS})
endif()

# Build dependencies
add_dependencies(sdv_dbc_util CompileCoreIDL)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_dbc_util PARENT_SCOPE)
