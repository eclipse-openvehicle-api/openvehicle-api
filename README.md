# This is the latest news about 'Eclipse Open Vehicle API'
#
# for more information see documentation: https://eclipse.dev/openvehicle-api/

The current state is a pre-investigation status.

Feel free to make your first personal experiences after installing a c++ compiler & cmake, compiling the framework, run the examples and then develop and play around with your own 'complex services' (also called 'application' or 'vehicle function').


# More Readme files

How to run tests:

Clicking [this link](./tests/README.md) Test README.md

How to run can socket tests:

Clicking [this link](./tests/unit_tests/can_com_tests/README.md) can_com_tests README.md

How to run manual silkit can socket tests:

Clicking [this link](./tests/manual_tests/silkit_can_com_tests/README.md) can_com_tests README.md

# Prerequisites

- Visual Studio Code or Visual Studio 2019 or higher
- Git
- CMake (at least version 3.20)
- GCC Compiler (version 14.2.0)

# Build Open Vehicle API Framework

## With Visual Studio Extension:

- Open Visual Studio Code.
- Go to File > Open Folder... and select 'your-project-directory'
- Replace 'your-project-directory' with the path to your project directory.
- In the CMake extension go to Configure section and select the required Preset.
- In the CMake extension go to Build and trigger a build
- After the compilation is complete, you can run the executable generated in the build directory. The executable can be found in 'your-project-directory'/build/'selected-compiler-platform'/bin
- During the build also the tests are included. The tests can be found in 'your-project-directory'/build/'selected-compiler-platform'/tests/bin

## Without Visual Studio Extension

- Navigate to the Project Directory: 
- In the terminal, type cd 'your-project-directory' and press Enter.
- Replace 'your-project-directory' with the path to your project directory.
- In the terminal, type cmake . --preset 'your-preset-name' and press Enter.
- Replace 'your-preset-name' with the preset you want to use.
- e.g. cmake . --preset gcc_w64_unix_debug.
- The available presets can be found in file CMakePresets.json
- This will configure the project and generate the necessary build files.
- In the terminal, type cmake --build . --preset 'your-preset-name' and press Enter.
- Replace 'your-preset-name' with the preset you want to use.
- e.g. cmake --build . --preset gcc_w64_unix_debug.
- The available presets can be found in file CMakePresets.json
- This will compile the project using the selected compiler.
- After the compilation is complete, you can run the executable generated in the build directory. The executable can be found in 'your-project-directory'/build/'selected-compiler-platform'/bin
- During the build also the tests are included. The tests can be found in 'your-project-directory'/build/'selected-compiler-platform'/tests/bin

# Setup Your Working Space

It is recommended to setup own working space after compiling the core framework of Open Vehicle API Framework.

First cretes the environment variables:

## On Windows

to add the environment variables, open "System Settings" and then "Environment Variables".
Add the following variables to either the user or the system settings:

| Variable name             | Location                                                                                       |
| ------------------------- | ---------------------------------------------------------------------------------------------- |
| SDV_FRAMEWORK_RUNTIME     | bin directory to the sdv_core.exe and sdv_iso.exe as well as the .sdv framework components.    |
| SDV_COMPONENT_INSTALL     | installation directory for the components                                                      |
| SDV_FRAMEWORK_DEV_TOOLS   | bin directory to the sdv_idl_compiler.exe and sdv_packager.exe.                                |
| SDV_FRAMEWORK_DEV_INCLUDE | export directory with interfaces and support sub-directory containing the framework interface. |

## On Linux

- Create a .env File
- Inside your home directory (or any consistent location in the container), create a file named .env
- Example: nano ~/.env or code ~/.env
- Add your environment variables in .env file like mentioned below:

```
# Set SDV Vehicle API environment variables
export SDV_FRAMEWORK_RUNTIME=~/share/vapi/build/bin
export SDV_COMPONENT_INSTALL=~/share/vapi/build/bin
export SDV_FRAMEWORK_DEV_TOOLS=~/share/vapi/build/bin
export SDV_FRAMEWORK_DEV_INCLUDE=~/share/vapi/export
```

- Use export so they are available to child processes (like build tools or VS Code terminals)
- Edit your shell profile file — usually ~/.bashrc or ~/.profile (depending on your shell):
- nano ~/.bashrc or code ~/.bashrc
- Add this line at the end:

```
# Load custom environment variables
if [ -f ~/.env ]; then
source ~/.env
fi
```

- This ensures that every time a new shell starts (including VS Code terminals), your environment variables are loaded.
- To Apply the Changes Immediately
- source ~/.bashrc
- you can check whether the environment variables are set properly: echo $<environment_variable>

## Copy the tmp_project folder

Copy the tmp_project folder to any place you like. The tmp_project folder contains a CMakeLists.txt file and a subfolder already. Start your project directly in the subfolder. Use tmp_project folder as root folder.
