param(
    [Parameter(Mandatory=$true)]
    [string]$SourceDir,

    [Parameter(Mandatory=$true)]
    [string]$BuildDir
)

$ErrorActionPreference = "Stop"

# フルパス化
$SourceDir = (Resolve-Path $SourceDir).Path
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}
$BuildDir = (Resolve-Path $BuildDir).Path

Write-Host "Source: $SourceDir"
Write-Host "Build : $BuildDir"

# CMake存在確認
try {
    cmake --version | Out-Null
} catch {
    throw "CMake is not installed or not in PATH"
}

# ============================
# CMake Generate
# ============================

$generator = "Visual Studio 18 2026"
$toolset   = "v145"

Write-Host "Generating Visual Studio solution..."
Write-Host "Generator : $generator"
Write-Host "Toolset   : $toolset"

cmake -S "$SourceDir" -B "$BuildDir" -G "$generator" -T "$toolset" -A x64

if ($LASTEXITCODE -ne 0) {
    throw "CMake generation failed (toolset=$toolset)"
}

Write-Host "Solution generated successfully."