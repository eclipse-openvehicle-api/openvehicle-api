<#
.SYNOPSIS
    PowerShell script to loop a specific test executable multiple times to catch intermittent failures.

.DESCRIPTION
    Usage: .\loop_test_runner.ps1 <TestName> <NumberOfRuns>
    Example: .\loop_test_runner.ps1 UnitTest_SharedMemConnectTests.exe 1000
#>

param (
    [Parameter(Mandatory = $true)]
    [string]$TestName,

    [Parameter(Mandatory = $true)]
    [int]$NumberOfRuns
)

# Build the full path to the test executable
$TargetTest = Join-Path "..\build\tests\bin" $TestName

# Check if the test executable exists
if (Test-Path $TargetTest) {
    Write-Host "`nStarting looped execution of $TargetTest to catch intermittent failures..." -ForegroundColor Cyan

    for ($i = 1; $i -le $NumberOfRuns; $i++) {
        Write-Host "`nRun #$i" -ForegroundColor Yellow
        & $TargetTest
        if ($LASTEXITCODE -ne 0) {
            Write-Host "`nTest failed on run #$i" -ForegroundColor Red
            break
        }
    }
} else {
    Write-Host "Target test $TargetTest not found!" -ForegroundColor Red
    exit 1
}