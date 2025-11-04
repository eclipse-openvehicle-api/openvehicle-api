# Generating an overview of the CMake dependencies:
## Get graphViz (https://graphviz.org/)
## make sure that the folder of the executables is in the PATH if you use the zip package
##
## Create an intermediate folder in the root directory and step into it
### mkdir build
### cd build
##
## Make sure that the CMakeGraphVizOptions.cmake file is in the root directory if you want to use
## customization like including custom cmake targets
##
## Create the .dot file for the project
### cmake --graphviz=../graphviz/example.dot ..
##
## Create a picture (all.png) of all dependencies of all targets
### dot -Tpng ../graphviz/example.dot -o ../all.png
##
## Create a picture (target.png) ol all dependencies of one specific target <target>
### dot -Tpng ../graphviz/example.dot.<target> -o ../target.png

set(GRAPHVIZ_CUSTOM_TARGETS TRUE)
set(GRAPHVIZ_IGNORE_TARGETS Winmm GTest::GTest Rpcrt4.lib Ws2_32 gmock gmock_main gtest_main gtest)