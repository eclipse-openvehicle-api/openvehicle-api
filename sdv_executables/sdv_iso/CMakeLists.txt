# Define project
project(sdv_iso VERSION 1.0 LANGUAGES CXX)

# build test application
add_executable(sdv_iso main.cpp)
if (WIN32)
    target_link_libraries(sdv_iso ${CMAKE_THREAD_LIBS_INIT} Ws2_32 Winmm Rpcrt4.lib)
else()
    target_link_libraries(sdv_iso ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} rt)
endif()

# Build dependencies
add_dependencies(sdv_iso CompileCoreIDL)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_iso PARENT_SCOPE)
