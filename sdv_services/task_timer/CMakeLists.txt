# Define project
project(task_timer VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(task_timer SHARED "tasktimer.h" "tasktimer.cpp")

target_link_options(task_timer PRIVATE)
if (WIN32)
target_link_libraries(task_timer ${CMAKE_THREAD_LIBS_INIT} Winmm.lib)
elseif (UNIX)
target_link_libraries(task_timer ${CMAKE_THREAD_LIBS_INIT} rt)
endif()
set_target_properties(task_timer PROPERTIES PREFIX "")
set_target_properties(task_timer PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(task_timer CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} task_timer PARENT_SCOPE)


# Define project
project(simulation_task_timer VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(simulation_task_timer SHARED "simulationtasktimer.h" "simulationtasktimer.cpp")

target_link_options(simulation_task_timer PRIVATE)
if (WIN32)
target_link_libraries(simulation_task_timer ${CMAKE_THREAD_LIBS_INIT} Winmm.lib)
elseif (UNIX)
target_link_libraries(simulation_task_timer ${CMAKE_THREAD_LIBS_INIT} rt)
endif()
set_target_properties(simulation_task_timer PROPERTIES PREFIX "")
set_target_properties(simulation_task_timer PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(simulation_task_timer CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} task_timer simulation_task_timer PARENT_SCOPE)
