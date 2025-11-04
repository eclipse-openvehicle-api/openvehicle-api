# PowerShell Script to execute the tests on Windows
# Save this script as run_tests_on_windows.ps1
# Run the script with: .\run_tests_on_windows.ps1

# Set the paths to the executables
$runTests = Get-ChildItem -Path "bin" -Filter "*.exe" | Where-Object {
    $_.Name -like "ComponentTest_*" -or $_.Name -like "UnitTest_*"
} | ForEach-Object {
    $_.FullName
}

# Array to hold failed tests
$failedTests = @()

# Run all tests and capture output
foreach ($testExecutable in $runTests) {
    if (Test-Path $testExecutable) {
        $output = & $testExecutable --gtest_output=xml:../bin/$(Split-Path -Leaf $testExecutable).xml 2>&1
        Write-Output $output
        if ($output -match "FAILED") {
            $failedTests += $testExecutable
        }
    } else {
        Write-Output "Executable $testExecutable not found!"
    }
}

# List all failed tests at the end
if ($failedTests.Count -ne 0) {
    Write-Host "`nFailed tests:" -ForegroundColor Red
    foreach ($test in $failedTests) {
        Write-Host $test -ForegroundColor Red
    }
} else {
    Write-Host "`nAll tests passed!" -ForegroundColor Green

}
