# SilKit CAN Communication Tests
This guide provides instructions on how to execute the manual tests for the SilKit CAN communication system, including the `can_reader` and `can_writer` components.

## Prerequisites
Before running the tests, ensure the following:

- Operating System: Ubuntu (or a Linux distribution with gnome-terminal installed) **or Windows**.
- Dependencies:
  - `sil-kit-registry` and `sil-kit-system-controller` binaries are available in the directory: `<...>/vapi-cpp-vehicle-api-platform/build/<compiler_version>/_deps/silkit-src/SilKit/bin/`
  - gnome-terminal is installed on your system (Linux only).
- Build: The project has been built successfully, and the binaries for can_reader and can_writer are available in the directory: `<...>/vapi-cpp-vehicle-api-platform/build/<compiler_version>/tests/bin/`

## Test Components
The tests involve the following components:

- `sil-kit-registry`: Manages the SilKit network.
- `sil-kit-system-controller`: Controls the lifecycle of SilKit participants.
- `can_writer`: Sends CAN messages to the network.
- `can_reader`: Receives CAN messages from the network.
  
## Instructions

### Linux
1. **Start the SilKit Utilities**
- Navigate to the directory containing the `run_silkit_util.sh` script:
  ```bash
  cd <...>/vapi-cpp-vehicle-api-platform/build/<compiler_version>/tests/manual_tests/
  ```

- Make the script executable (if not already):
    ```bash
    chmod +x run_silkit_util.sh
    ```
- Run the script to start the SilKit utilities:
    ```bash
    ./run_silkit_util.sh
    ```
    This will open two new terminal windows:
    - One running `sil-kit-registry`.
    - Another running `sil-kit-system-controller` with the arguments `can_reader can_writer`.

2. **Run the CAN Writer**
In a new terminal, navigate to the directory containing the can_writer binary:
    ```bash
    cd <...>/vapi-cpp-vehicle-api-platform/build/<compiler_version>/tests/bin/
    ```

   - Run the can_writer binary: 
   ```bash
   ./can_writer
   ```

    The `can_writer` will send different CAN messages to the network. Each message will be logged in the terminal.

3. **Run the CAN Reader**
In another terminal, navigate to the directory containing the can_reader binary:
    ```bash
    cd <...>/vapi-cpp-vehicle-api-platform/build/<compiler_version>/tests/bin/
    ```

   - Run the `can_reader` binary: 
   ```bash
   ./can_reader
   ```

    The `can_reader` will receive and log the CAN messages sent by the `can_writer`. Each message will be logged in the terminal.

### Windows
1. **Start the SilKit Utilities**
- Before running the executables, copy the `SilKit.dll` and `SilKitd.dll` DLLs from the SilKit source directory `<..>\_deps\silkit-src\SilKit\bin` to the `<..>\test\bin` directory.

- Open PowerShell or Command Prompt and navigate to the directory containing the `run_silkit_util.bat` script (if available):
  ```bat
  cd <...>\vapi-cpp-vehicle-api-platform\build\<compiler_version>\tests\manual_tests
  ```
- Run the script to start the SilKit utilities:
  ```bat
  run_silkit_util.bat
  ```
  This will open two new windows:
  - One running `sil-kit-registry`.
  - Another running `sil-kit-system-controller` with the arguments `can_reader can_writer`.

  **If the script is not available, you can start the utilities manually:**
  - Open two Command Prompt or PowerShell windows:
    - In the first window, navigate to the SilKit bin directory and run:
      ```bat
      cd <...>\vapi-cpp-vehicle-api-platform\build\<compiler_version>\_deps\silkit-src\SilKit\bin
      sil-kit-registry.exe
      ```
    - In the second window, run:
      ```bat
      sil-kit-system-controller.exe can_reader can_writer
      ```

1. **Run the CAN Writer**
- Open a new Command Prompt or PowerShell window and navigate to the directory containing the can_writer binary:
  ```bat
  cd <...>\vapi-cpp-vehicle-api-platform\build\<compiler_version>\tests\bin
  can_writer.exe
  ```

  The `can_writer` will send different CAN messages to the network. Each message will be logged in the terminal.

3. **Run the CAN Reader**
- Open another Command Prompt or PowerShell window and navigate to the directory containing the can_reader binary:
  ```bat
  cd <...>\vapi-cpp-vehicle-api-platform\build\<compiler_version>\tests\bin
  can_reader.exe
  ```

  The `can_reader` will receive and log the CAN messages sent by the `can_writer`. Each message will be logged in the terminal.

## Expected Output
- **CAN Writer:** Logs the messages being sent, including the CAN ID, data length, and data content in the terminal.

- **CAN Reader:** Logs the messages being received, including the CAN ID, data length, and data content in the terminal.

## Troubleshooting
- Binaries Not Found: Ensure the `sil-kit-registry`, `sil-kit-system-controller`, `can_writer`, and `can_reader` binaries are built and located in the correct directories.

- **Terminal Emulator: The script uses gnome-terminal on Linux. On Windows, PowerShell is used. If you use a different terminal emulator, modify the scripts accordingly.**

- Network Issues: Ensure the RegistryURI in the configuration matches the address of the sil-kit-registry.

## Additional Notes
- The `can_writer` and `can_reader` configurations are hardcoded in their respective source files. If you need to modify the configurations (e.g., participant names, network names, or registry URI), update the source code and rebuild the binaries.
- The tests are designed to run in synchronous mode (SyncMode = true). Ensure this setting is consistent across all components.