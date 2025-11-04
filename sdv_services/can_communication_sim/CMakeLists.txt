# Define project
project(can_com_sim VERSION 1.0 LANGUAGES CXX)

# Define target
add_library(can_com_sim SHARED "can_com_sim.h" "can_com_sim.cpp")
target_link_options(can_com_sim PRIVATE)
target_link_libraries(can_com_sim ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(can_com_sim PROPERTIES PREFIX "")
set_target_properties(can_com_sim PROPERTIES SUFFIX ".sdv")

# Build dependencies
add_dependencies(can_com_sim CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} can_com_sim PARENT_SCOPE)
