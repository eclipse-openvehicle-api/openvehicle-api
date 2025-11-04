# Define project
project(sdv_local_shutdown VERSION 1.0 LANGUAGES CXX)

# build test application
add_executable(sdv_local_shutdown
    "main.cpp"
    "../../sdv_services/core/local_shutdown_request.h"
    )
if (WIN32)
    target_link_libraries(sdv_local_shutdown ${CMAKE_THREAD_LIBS_INIT} Ws2_32 Winmm Rpcrt4.lib)
else()
    target_link_libraries(sdv_local_shutdown ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} rt)
endif()

# Build dependencies
add_dependencies(sdv_local_shutdown core_services)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_local_shutdown PARENT_SCOPE)
