[CmdletBinding()]
param(
    [ValidateSet('1','2','3','3.1','all','tests')][string]$Practice = 'all',
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [string]$ToolchainBin,
    [switch]$Test
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
$toolchain = Get-CourseToolchainBin $ToolchainBin
$projectRoot = Split-Path -Parent $PSScriptRoot
$oldPath = $env:PATH
try {
    $env:PATH = $toolchain + ';' + $env:PATH
    $cmake = (Get-Command cmake.exe -ErrorAction Stop | Select-Object -First 1).Source
    Get-Command ninja.exe -ErrorAction Stop | Out-Null
    $buildDirectory = 'build/' + $Configuration.ToLowerInvariant()
    Push-Location -LiteralPath $projectRoot
    try {
        & $cmake -S . -B $buildDirectory -G Ninja "-DCMAKE_BUILD_TYPE=$Configuration" "-DCMAKE_CXX_COMPILER=$toolchain/g++.exe"
        if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed. See README.md.' }
        $buildArgs = @('--build', $buildDirectory, '--parallel', '2')
        if ($Practice -eq 'tests') { $buildArgs += @('--target', 'course_checks') }
        elseif ($Practice -ne 'all') { $buildArgs += @('--target', ('practice0' + $Practice.Replace('.','_'))) }
        & $cmake @buildArgs
        if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
        if ($Test) {
            # CTest smoke tests also need the three practice executables.
            & $cmake --build $buildDirectory --parallel 2
            if ($LASTEXITCODE -ne 0) { throw 'Test target build failed.' }
            & ctest --test-dir $buildDirectory --output-on-failure
            if ($LASTEXITCODE -ne 0) { throw 'Tests failed.' }
        }
    } finally { Pop-Location }
} finally { $env:PATH = $oldPath }
