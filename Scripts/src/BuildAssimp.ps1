# assimp を clone → パッチ適用 → CMake → ビルド → ヘッダーコピー まで行う
# setup.bat から DirectXGame ディレクトリをカレントにして呼び出される前提
param(
    [string]$Tag = "v6.0.5"
)

$ErrorActionPreference = "Stop"

$scriptDir = $PSScriptRoot
$cloneDir  = "externals\generated\Project\assimp_clone"
$buildDir  = "externals\generated\Project\assimp"

function Invoke-Step {
    param([string]$Script, [string[]]$Arguments)
    & powershell -ExecutionPolicy Bypass -NoProfile -File (Join-Path $scriptDir $Script) @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Script failed (exit code $LASTEXITCODE)"
    }
}

# ============================
# Clone
# ============================
Write-Host "===== Clone assimp ($Tag) ====="
git -c core.longpaths=true clone --branch $Tag --depth 1 --recurse-submodules --shallow-submodules https://github.com/assimp/assimp.git "$cloneDir/"
if ($LASTEXITCODE -ne 0) {
    throw "git clone failed"
}

# ============================
# Patch: pugixml の dllexport を無効化
# ============================
# assimp 同梱の pugiconfig.hpp は _WIN32 で無条件に
#   #define PUGIXML_API __declspec(dllexport)
# としている。静的ライブラリとしてビルドしてもこの指定が .obj に /EXPORT として残り、
# assimp をリンクした exe が pugixml の関数をエクスポートしてしまう。
# その結果、リンカが不要な <exe名>.lib / <exe名>.exp を出力するため、空定義に置き換える。
Write-Host "===== Patch pugiconfig.hpp (remove dllexport) ====="
$pugiConfig = Join-Path $cloneDir "contrib\pugixml\src\pugiconfig.hpp"
if (!(Test-Path $pugiConfig)) {
    throw "Not Found: $pugiConfig"
}

$content = [System.IO.File]::ReadAllText((Resolve-Path $pugiConfig).Path)
$pattern = '(?m)^(\s*#\s*define\s+PUGIXML_API)\s+__declspec\(dllexport\).*$'
if ($content -notmatch $pattern) {
    throw "pugiconfig.hpp: dllexport definition not found. Check the patch for assimp $Tag."
}
$content = [regex]::Replace($content, $pattern, '$1 // patched by BuildAssimp.ps1: no dllexport for static build')
[System.IO.File]::WriteAllText((Resolve-Path $pugiConfig).Path, $content)
Write-Host "Patched: $pugiConfig"

# ============================
# CMake / Build
# ============================
Invoke-Step "CMakeProject.ps1" @("-SourceDir", $cloneDir, "-BuildDir", $buildDir)
Invoke-Step "BuildExternals.ps1" @("-ProjectPath", "$buildDir\assimp.slnx", "-Platform", "x64")

# ============================
# Copy Header
# ============================
Write-Host "===== Copy Header ====="
Invoke-Step "CopyHeader.ps1" @("-SourceDir", "$cloneDir\include\assimp", "-DestDir", "externals\header\assimp")
Invoke-Step "CopyHeader.ps1" @("-SourceDir", "$buildDir\include\assimp", "-DestDir", "externals\header\assimp")

Write-Host "assimp build completed."
