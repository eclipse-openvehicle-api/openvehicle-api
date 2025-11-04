################################## ATTENTION #################################
# Shell Script to execute the tests on Linux                                 #
# Adjust the permission with: chmod +x loop_test_runner_linux.sh             #
# Run the script with: ./loop_test_runner_linux.sh <TestName> <NumberOfRuns> #
# or: bash ./loop_test_runner_linux.sh <TestName> <NumberOfRuns>             #
##############################################################################

#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <TestName> <NumberOfRuns>"
    echo "Example: $0 UnitTest_SharedMemConnectTests 1000"
    exit 1
fi

# Get the test name and number of runs from command-line arguments
TEST_NAME="$1"
MAX_RUNS="$2"

# Build the full path to the test executable
TARGET_TEST="../build/tests/bin/$TEST_NAME"

# Check if the test executable exists
if [ -f "$TARGET_TEST" ]; then
    echo -e "\nStarting looped execution of $TARGET_TEST to catch intermittent failures..."
    
    # Loop the test execution for the specified number of times
    for i in $(seq 1 $MAX_RUNS); do
        echo -e "\nRun #$i"
        $TARGET_TEST
        if [ $? -ne 0 ]; then
            echo -e "\nTest failed on run #$i"
            break
        fi
    done
else
    echo "Target test $TARGET_TEST not found!"
    exit 1
fi        