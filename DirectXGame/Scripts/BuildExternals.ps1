param(
    [string]$ProjectPath,
    [string]$Platform = "x64"
)

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (!(Test-Path $vswhere)) {
    throw "Not Found vswhere.exe"
}

$msbuildPath = & $vswhere `
    -latest `
    -prerelease `
    -requires Microsoft.Component.MSBuild `
    -find MSBuild\**\Bin\MSBuild.exe

if (-not $msbuildPath) {
    throw "Not Found MSBuild"
}

Write-Host "MSBuild Path: $msbuildPath"

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

Build-Project "Debug"
Build-Project "Release"

Write-Host "All builds succeeded."