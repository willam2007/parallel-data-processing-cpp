Set-StrictMode -Version Latest

function Get-CourseToolchainBin {
    param([string]$RequestedBin)
    if (-not $RequestedBin) { $RequestedBin = $env:CPP_TOOLCHAIN_BIN }
    if (-not $RequestedBin -and (Test-Path -LiteralPath 'C:\msys64\ucrt64\bin\g++.exe')) {
        $RequestedBin = 'C:\msys64\ucrt64\bin'
    }
    if (-not $RequestedBin) {
        $compilerCommand = Get-Command g++.exe -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($compilerCommand) { $RequestedBin = Split-Path -Parent $compilerCommand.Source }
    }
    if (-not $RequestedBin -or -not (Test-Path -LiteralPath (Join-Path $RequestedBin 'g++.exe'))) {
        throw 'MSYS2 GCC not found. Run scripts/setup-windows.ps1 or set CPP_TOOLCHAIN_BIN to ucrt64/bin.'
    }
    return (Resolve-Path -LiteralPath $RequestedBin).Path
}
