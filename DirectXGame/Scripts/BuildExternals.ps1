param(
    [string]$ProjectPath,
    [string]$Platform = "x64"
)

# --- vswhere のパス（通常ここにある） ---
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (!(Test-Path $vswhere)) {
    throw "vswhere.exe が見つかりません"
}

# --- Preview(Insider)のVisual StudioからMSBuildを探す ---
$msbuildPath = & $vswhere `
    -latest `
    -prerelease `
    -requires Microsoft.Component.MSBuild `
    -find MSBuild\**\Bin\MSBuild.exe

if (-not $msbuildPath) {
    throw "MSBuild が見つかりません（Preview環境含む）"
}

Write-Host "MSBuild Path: $msbuildPath"

# --- 共通ビルド関数 ---
function Build-Project {
    param(
        [string]$Configuration
    )

    Write-Host "===== Building: $Configuration | $Platform ====="

    & $msbuildPath $ProjectPath `
        /p:Configuration=$Configuration `
        /p:Platform=$Platform `
        /m `
        /verbosity:minimal

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed: $Configuration"
    }
}

# --- Debug / Release 両方ビルド ---
Build-Project "Debug"
Build-Project "Release"

Write-Host "All builds succeeded."