# Define project
project (sdv_control VERSION 1.0 LANGUAGES CXX)

# Add include directories
include_directories(../export)

# Define the executable
add_executable(sdv_control
    main.cpp
    "list_elements.cpp"
    "list_elements.h"
    "startup_shutdown.h"
    "startup_shutdown.cpp"
    "context.h"
    "print_table.h" "start_stop_service.cpp" "start_stop_service.h" "installation.h" "installation.cpp")

target_link_libraries(sdv_control ${CMAKE_DL_LIBS})

# Build dependencies
add_dependencies(sdv_control core_services)

# Appending the executable to the service list
set(SDV_Executable_List ${SDV_Executable_List} sdv_control PARENT_SCOPE)
