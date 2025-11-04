@echo off
REM Run sil-kit-registry in a new Command Prompt window
start cmd /k "..\..\_deps\silkit-src\SilKit\bin\sil-kit-registry.exe"

REM Run sil-kit-system-controller with arguments in another new Command Prompt window
start cmd /k "..\..\_deps\silkit-src\SilKit\bin\sil-kit-system-controller.exe can_reader can_writer"
