# Define project
project(sdv_core VERSION 1.0 LANGUAGES CXX)

# build test application
add_executable(sdv_core main.cpp)
if (WIN32)
    target_link_libraries(sdv_core ${CMAKE_THREAD_LIBS_INIT} Ws2_32 Winmm Rpcrt4.lib)
else()
    target_link_libraries(sdv_core ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} rt)
endif()

# Build dependencies
add_dependencies(sdv_core core_services)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_core PARENT_SCOPE)
