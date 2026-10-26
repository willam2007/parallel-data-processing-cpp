[CmdletBinding()]
param([string]$MsysRoot = 'C:\msys64')
$ErrorActionPreference = 'Stop'
$bash = Join-Path $MsysRoot 'usr/bin/bash.exe'
if (-not (Test-Path -LiteralPath $bash)) {
    throw 'Install MSYS2 first: winget install --id MSYS2.MSYS2 --exact. Then update it with pacman -Syu. See README.md.'
}
$oldSystem = $env:MSYSTEM
try {
    $env:MSYSTEM = 'UCRT64'
    & $bash -lc 'pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-libgomp mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja'
    if ($LASTEXITCODE -ne 0) { throw 'Package installation failed. Update MSYS2 with pacman -Syu and retry.' }
} finally { $env:MSYSTEM = $oldSystem }
$codeCommand = Get-Command code -ErrorAction SilentlyContinue | Select-Object -First 1
if ($codeCommand) {
    & $codeCommand.Source --install-extension ms-vscode.cpptools
    if ($LASTEXITCODE -ne 0) { throw 'VS Code extension installation failed.' }
}
Write-Host 'Ready. Run: powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Test'
