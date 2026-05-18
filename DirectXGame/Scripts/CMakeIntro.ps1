# ============================
# CMake Setup Script (Robust)
# ============================

$ErrorActionPreference = "Stop"

# ============================
# Utility
# ============================

function Find-CMakePath {

    Write-Host "Attempting to find CMake executable..."

    # ① レジストリ
    try {
        $reg = Get-ItemProperty "HKLM:\SOFTWARE\Kitware\CMake" -ErrorAction Stop
        $installDir = $reg.InstallDir
        $exePath = Join-Path $installDir "bin\cmake.exe"
        Write-Host "Checking registry path: $exePath"

        if (Test-Path $exePath) {
            Write-Host "Found in registry: $exePath"
            return $exePath
        }
    } catch {}

    Write-Host "Not found in registry."

    # ② Program Files
    $paths = @(
        "C:\Program Files\CMake\bin\cmake.exe",
        "C:\Program Files (x86)\CMake\bin\cmake.exe"
    )

    foreach ($p in $paths) {
        if (Test-Path $p) {
            Write-Host "Found in Program Files: $p"
            return $p
        }
    }
    Write-Host "Not found in standard Program Files locations."

    # ③ Visual Studio内蔵（これが今回重要）
    $vsPath = "C:\Program Files\Microsoft Visual Studio"
    if (Test-Path $vsPath) {
        $found = Get-ChildItem $vsPath -Recurse -Filter cmake.exe -ErrorAction SilentlyContinue |
                 Select-Object -First 1

        if ($found) {
            return $found.FullName
        }
    }
    Write-Host "Not found in Visual Studio locations."

    # ④ PATH fallback
    $cmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmd) {
        return $cmd.Source
    }

    Write-Host "CMake executable not found."
    return $null
}

function Get-CMakeVersion {
    Write-Host "Getting CMake version..."
    try {
        $cmakePath = Find-CMakePath
        Write-Host "Finished Find-CMakePath: $cmakePath"
        if (-not $cmakePath) { 
            Write-Host "CMake executable not found for version check."
            return $null 
        }

        $versionOutput = & "$cmakePath" --version
        $firstLine = $versionOutput.Split("`n")[0]
        Write-Host "Version output: $firstLine"
        if ($firstLine -match "cmake version ([\d\.]+)") {
            return $matches[1]
        }
    } catch {}
    return $null
}

function Get-LatestCMakeInfo {
    $url = "https://api.github.com/repos/Kitware/CMake/releases/latest"
    $response = Invoke-RestMethod -Uri $url

    $version = $response.tag_name.TrimStart("v")

    $asset = $response.assets | Where-Object {
        $_.name -like "*windows-x86_64.msi"
    } | Select-Object -First 1

    if (-not $asset) {
        throw "Failed to find Windows installer asset."
    }

    return @{
        Version = $version
        Url     = $asset.browser_download_url
    }
}

function Add-ToSystemPath {
    param([string]$newPath)

    $machinePath = [Environment]::GetEnvironmentVariable("Path", "Machine")

    if ($machinePath -notlike "*$newPath*") {
        Write-Host "Adding to system PATH..."

        $updatedPath = "$machinePath;$newPath"
        [Environment]::SetEnvironmentVariable("Path", $updatedPath, "Machine")
    }

    # 現在セッションにも反映
    if ($env:Path -notlike "*$newPath*") {
        $env:Path += ";$newPath"
    }

    Write-Host "CMake installed. Please restart .bat file to use the updated PATH."
    pause
    exit 0
}

# ============================
# Install / Update
# ============================

function Install-CMake {
    param(
        [string]$url,
        [string]$version
    )

    Write-Host "Installing CMake $version ..."

    $installerPath = "$env:TEMP\cmake_$version.msi"

    Invoke-WebRequest -Uri $url -OutFile $installerPath

    Start-Process msiexec.exe -Wait -ArgumentList "/i `"$installerPath`" /quiet /norestart"

    Remove-Item $installerPath -Force

    # PATH登録（確実にやる）
    Add-ToSystemPath "C:\Program Files\CMake\bin"

    Write-Host "CMake installation completed."
}

# ============================
# Main
# ============================

Write-Host "=== CMake Setup ==="

$currentVersion = Get-CMakeVersion

if ($currentVersion) {
    Write-Host "Current version: $currentVersion"
} else {
    Write-Host "CMake not found."
}

$latestInfo = Get-LatestCMakeInfo
$latestVersion = $latestInfo.Version
$latestUrl     = $latestInfo.Url

Write-Host "Latest version:  $latestVersion"

if (-not $currentVersion) {
    Install-CMake -url $latestUrl -version $latestVersion
}
elseif ($currentVersion -ne $latestVersion) {
    Write-Host "Updating CMake..."
    Install-CMake -url $latestUrl -version $latestVersion
}
else {
    Write-Host "CMake is up to date."
}

# 最終確認
$finalVersion = Get-CMakeVersion

if ($finalVersion) {
    Write-Host "Ready: CMake $finalVersion"
} else {
    Write-Host "Warning: CMake installed but not detected."
}