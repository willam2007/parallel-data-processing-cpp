[CmdletBinding()]
param(
    [ValidateSet('1','2','3','3.1')][string]$Practice = '3',
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [UInt64]$Size = 0,
    [ValidateRange(0,3)][int]$Task = 2,
    [ValidateRange(1,2147483647)][int]$Repeats = 5,
    [string]$ToolchainBin,
    [switch]$NoBuild
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'common.ps1')
$toolchain = Get-CourseToolchainBin $ToolchainBin
$projectRoot = Split-Path -Parent $PSScriptRoot
if (-not $NoBuild) {
    & (Join-Path $PSScriptRoot 'build.ps1') -Practice $Practice -Configuration $Configuration -ToolchainBin $toolchain
}
if ($Size -eq 0) {
    if ($Practice -ne '3') { $Size = 1048576 }
    elseif ($Task -eq 3) { $Size = 256 }
    else { $Size = 2048 }
}
$programArgs = @('--size', [string]$Size)
if ($Practice -eq '3') { $programArgs += @('--task', [string]$Task, '--repeats', [string]$Repeats) }
if ($Practice -eq '3.1') { $programArgs = @() }
$binary = Join-Path $projectRoot ('build/' + $Configuration.ToLowerInvariant() + '/practice0' + $Practice.Replace('.','_') + '.exe')
if (-not (Test-Path -LiteralPath $binary)) { throw 'Executable missing. Run scripts/build.ps1 first.' }
$oldPath = $env:PATH
try {
    $env:PATH = $toolchain + ';' + $env:PATH
    & $binary @programArgs
    if ($LASTEXITCODE -ne 0) { throw 'Practice exited with an error.' }
} finally { $env:PATH = $oldPath }
