REM /********************************************************************************
REM * Copyright (c) 2025-2026 ZF Friedrichshafen AG
REM *
REM * This program and the accompanying materials are made available under the 
REM * terms of the Apache License Version 2.0 which is available at
REM * https://www.apache.org/licenses/LICENSE-2.0
REM *
REM * SPDX-License-Identifier: Apache-2.0 
REM ********************************************************************************/
REM 
REM 
REM 
@echo off
REM Run sil-kit-registry in a new Command Prompt window
start cmd /k "..\..\_deps\silkit-src\SilKit\bin\sil-kit-registry.exe"

REM Run sil-kit-system-controller with arguments in another new Command Prompt window
start cmd /k "..\..\_deps\silkit-src\SilKit\bin\sil-kit-system-controller.exe can_reader can_writer"
