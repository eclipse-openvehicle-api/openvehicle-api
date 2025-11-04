# Define project
project (sdv_vss_util VERSION 1.0 LANGUAGES CXX)

# Add include directories
include_directories(../export)

# Define the executable
add_executable(sdv_vss_util
    main.cpp
    csv_file_reader.h
    csv_file_reader.cpp
    vss_vd_generator.h
    vss_vd_generator.cpp
    vss_bs_generator.h
    vss_bs_generator.cpp
    vss_helper.cpp
    vss_helper.h
    vss_coding.cpp
    vss_coding.h    
    vss_vd_codingrx.cpp
    vss_vd_codingrx.h    
    vss_bs_codingrx.cpp
    vss_bs_codingrx.h    
    vss_vd_codingtx.cpp
    vss_vd_codingtx.h    
    vss_bs_codingtx.cpp
    vss_bs_codingtx.h    
    codegen_base.cpp)

if (WIN32)
target_link_libraries(sdv_vss_util ${CMAKE_DL_LIBS} Rpcrt4.lib)
else()
target_link_libraries(sdv_vss_util ${CMAKE_DL_LIBS})
endif()

# Build dependencies
add_dependencies(sdv_vss_util CompileCoreIDL)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_vss_util PARENT_SCOPE)
