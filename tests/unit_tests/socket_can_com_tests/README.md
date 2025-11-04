# Virtual CAN Interface Setup and Testing

This guide explains how to set up virtual CAN interfaces (`vcan0` and `vcan1`) on a Linux system and how to run tests using these interfaces.

## Prerequisites

- A Linux system with `iproute2` and `can-utils` installed.
- Root or sudo privileges.

## Setup Virtual CAN Interfaces

1. **Load the `vcan` kernel module:**

    ```bash
    sudo modprobe vcan
    ```

2. **Create the virtual CAN interfaces:**

    ```bash
    sudo ip link add dev vcan0 type vcan
    sudo ip link add dev vcan1 type vcan
    sudo ip link add dev vcan2 type vcan
    sudo ip link add dev vcan3 type vcan
    sudo ip link add dev vcan4 type vcan
    ```

3. **Bring the virtual CAN interfaces online:**

    ```bash
    sudo ip link set up vcan0
    sudo ip link set up vcan1
    sudo ip link set up vcan2
    sudo ip link set up vcan3
    sudo ip link set up vcan4
    ```

4. **Verify the virtual CAN interfaces:**

    ```bash
    ip addr | grep "vcan"
    ```

    You should see `vcan0` and `vcan1` listed as available interfaces.

5. **Remove the virtual CAN interfaces:**
 
    ```bash
    sudo ip link set down vcan0
    sudo ip link delete vcan0
    ```

## Running Tests

1. **Listen for CAN messages on `vcan0`:**
 
    ```bash
    candump vcan0
    ```
 
2. **Send a CAN message on `vcan0` (from another terminal):**
 
    ```bash
    cansend vcan0 123#DEADBEEF
    ```
    You should see the message `123#DEADBEEF` appear on `vcan0`.

## Explanation

This setup uses virtual CAN interfaces (`vcan0` and `vcan1`) to simulate a CAN bus environment. This is useful for testing CAN applications without the need for physical CAN hardware.

**After virtual can is ready uncomment this line `#add_subdirectory(manual_tests/socket_can_com_tests)` from `vapi-cpp-vehicle-api-platform/tests/CMakeLists.txt` and then reconfigure and build the project to run the tests.**

## Additional Resources

- SocketCAN Documentation
- [can-utils GitHub Repository](https://github.com/linux-can/can-utils)