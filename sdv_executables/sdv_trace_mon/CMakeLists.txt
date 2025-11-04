# Define project
project (sdv_trace_mon VERSION 1.0 LANGUAGES CXX)

# Add include directories
include_directories(../export)

# Define the executable
add_executable(sdv_trace_mon
    main.cpp
)

# Link target
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(sdv_trace_mon ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
    if (WIN32)
        target_link_libraries(sdv_trace_mon Ws2_32 Winmm Rpcrt4.lib)
    else()
        target_link_libraries(sdv_trace_mon ${CMAKE_DL_LIBS} rt)
    endif()
else()
    target_link_libraries(sdv_trace_mon Rpcrt4.lib)
endif()

# Build dependencies
add_dependencies(sdv_trace_mon core_services)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_trace_mon PARENT_SCOPE)
