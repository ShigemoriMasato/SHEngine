param(
[string]$CMakeVersion = "3.30.0"
)

$ErrorActionPreference = "Stop"

function Test-CMakeInstalled {
$cmd = Get-Command cmake -ErrorAction SilentlyContinue
return $null -ne $cmd
}

function Install-CMake {
param([string]$Version)

```
Write-Host "CMake が見つからないためインストールを開始します..."

$url = "https://github.com/Kitware/CMake/releases/download/v$Version/cmake-$Version-windows-x86_64.msi"
$installer = "$env:TEMP\cmake.msi"

Write-Host "Download: $url"
Invoke-WebRequest -Uri $url -OutFile $installer

Write-Host "Install..."
Start-Process msiexec.exe -Wait -ArgumentList "/i `"$installer`" /qn"

Remove-Item $installer -Force
```

}

function Get-CMakePath {
$default = "C:\Program Files\CMake\bin"
if (Test-Path $default) {
return $default
}

```
# 念のため探索
$found = Get-ChildItem "C:\Program Files" -Recurse -Filter cmake.exe -ErrorAction SilentlyContinue | Select-Object -First 1
if ($found) {
    return $found.DirectoryName
}

return $null
```

}

function Add-ToPath {
param([string]$PathToAdd)

```
$currentUserPath = [Environment]::GetEnvironmentVariable("Path", "User")

if ($currentUserPath -like "*$PathToAdd*") {
    Write-Host "PATH は既に設定済みです"
    return
}

Write-Host "PATH に追加します: $PathToAdd"

$newPath = "$currentUserPath;$PathToAdd"
[Environment]::SetEnvironmentVariable("Path", $newPath, "User")

# 現在のセッションにも反映
$env:Path += ";$PathToAdd"
```

}

# --- メイン処理 ---

if (Test-CMakeInstalled) {
Write-Host "CMake は既にインストールされています"
} else {
Install-CMake -Version $CMakeVersion
}

$cmakePath = Get-CMakePath

if (-not $cmakePath) {
Write-Error "CMake のパスが見つかりませんでした"
return
}

Add-ToPath -PathToAdd $cmakePath

Write-Host "=== 完了 ==="
Write-Host "cmake version:"
cmake --version
