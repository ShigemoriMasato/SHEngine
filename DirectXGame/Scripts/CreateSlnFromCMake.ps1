param(
    [Parameter(Mandatory = $true)]
    [string]$SourceDir,
    [Parameter(Mandatory = $true)]
    [string]$BuildDir,
    [string]$Generator = "Ninja",
    [string]$Platform  = "x64",
    [string]$Config    = "Release"
)

# MSYS排除
$env:PATH = ($env:PATH -split ';' | Where-Object { $_ -notlike "*msys64*" }) -join ';'

# MSVC強制
$env:CC  = "cl.exe"
$env:CXX = "cl.exe"

# --- ログ出力 ---
Write-Host "Building" $SourceDir
Write-Host "Output" $BuildDir

# --- パス解決 ---

try {
$SourceDir = (Resolve-Path $SourceDir).Path
} catch {
Write-Error "SourceDir が存在しません: $SourceDir"
return 1
}

if (!(Test-Path $BuildDir)) {
New-Item -ItemType Directory -Path $BuildDir | Out-Null
}
$BuildDir = (Resolve-Path $BuildDir).Path

# --- CMake存在確認 ---

$cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmakeCmd) {
Write-Error "cmake が見つかりません。PATHを確認してください。"
return 1
}

Write-Host "=== CMake Generate Start ==="
Write-Host "Source : $SourceDir"
Write-Host "Build  : $BuildDir"
Write-Host "Gen    : $Generator"
Write-Host "Plat   : $Platform"

# --- CMake実行 ---

Push-Location $BuildDir

$cmakeArgs = @(
$SourceDir,
"-G", $Generator
# "-A", $Platform
)

Write-Host "Args:"
$cmakeArgs | ForEach-Object { Write-Host "[$_]" }

& "C:\Program Files\CMake\bin\cmake.exe" @cmakeArgs
if ($LASTEXITCODE -ne 0) {
Write-Error "CMake Generate に失敗しました"
Pop-Location
return 1
}

# --- ソリューション確認 ---

$sln = Get-ChildItem -Path $BuildDir -Filter *.sln | Select-Object -First 1
if ($sln) {
Write-Host "=== SUCCESS ==="
Write-Host "Solution: $($sln.FullName)"
} else {
Write-Warning "slnファイルが見つかりませんでした"
}

Pop-Location
