############################## ATTENTION ##############################
# Shell Script to execute the tests on Linux                          #
# Adjust the permission with: chmod +x run_tests_on_linux.sh          #
# Run the script with: ./run_tests_on_linux.sh                        #
# or: bash ./run_tests_on_linux.sh                                    #
#######################################################################

#!/bin/bash

# Define the directory to search
directory="bin"

# Sammeln der ausführbaren Dateien in einem Array
IFS=$'\n' read -r -d '' -a RUN_TESTS < <(find bin -type f \( -name "ComponentTest_*" -o -name "UnitTest_*" \) -executable -exec sh -c 'for file; do [ "${file##*.}" = "$file" ] && echo "$file"; done' sh {} \; && printf '\0')

# Array to hold failed tests
FAILED_TESTS=()

# Run all tests and capture output
for TEST_EXECUTABLE in "${RUN_TESTS[@]}"; do
    if [ -f "$TEST_EXECUTABLE" ]; then
        OUTPUT=$($TEST_EXECUTABLE --gtest_output=xml:../bin/$(basename $TEST_EXECUTABLE).xml 2>&1)
        echo "$OUTPUT"
        if echo "$OUTPUT" | grep -q "FAILED"; then
            FAILED_TESTS+=("$TEST_EXECUTABLE")
        fi
    else
        echo "Executable $TEST_EXECUTABLE not found!"
    fi
done

# List all failed tests at the end
if [ ${#FAILED_TESTS[@]} -ne 0 ]; then
    echo -e "\n\e[31mFailed tests:\e[0m"
    for TEST in "${FAILED_TESTS[@]}"; do
        echo -e "\e[31m$TEST\e[0m"
    done
else
    echo -e "\n\e[32mAll tests passed!\e[0m"
fi
