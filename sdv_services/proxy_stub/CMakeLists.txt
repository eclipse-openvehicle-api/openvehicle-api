# Include cross-compilation toolchain file
include(../../cross-compile-tools.cmake)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Define project
project(core_prox_stub VERSION 1.0 LANGUAGES CXX)

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)

# Libary symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Include link to export directory of "sdv-System". Replace this with link to installed location.
include_directories(../../export)

# Include cross-compilation toolchain file
include(../../cross-compile-tools.cmake)

# BIG Object for GCC on MINGW
if (WIN32)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/bigobj)
else()
    add_compile_options(-Wa,-mbig-obj)
endif()
endif()

# Add the dynamic library
file(MAKE_DIRECTORY "../../export/interfaces/ps/")
file(TOUCH "../../export/interfaces/ps/proxystub.cpp")
add_library(core_ps SHARED "../../export/interfaces/ps/proxystub.cpp" )
target_link_options(core_ps PRIVATE)
target_link_libraries(core_ps ${CMAKE_THREAD_LIBS_INIT})

# Set extension to .sdv
set_target_properties(core_ps PROPERTIES PREFIX "")
set_target_properties(core_ps PROPERTIES SUFFIX ".sdv")

add_dependencies(core_ps CompileCoreIDL)

# Appending the service in the service list
set(SDV_Service_List ${SDV_Service_List} core_ps PARENT_SCOPE)
