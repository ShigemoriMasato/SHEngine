# ====================================================================
# CMake 4.3.2 自動インストールスクリプト (Windows 64bit用)
# ====================================================================

# エラーが発生したら処理を即座に中断する設定
$ErrorActionPreference = "Stop"

# 1. 設定情報の定義
$version = "4.3.2"
$url = "https://github.com/Kitware/CMake/releases/download/v$version/cmake-$version-windows-x86_64.zip"
$installParentDir = "C:\Program Files\CMake"                         # インストール先の親フォルダ
$installDir = Join-Path $installParentDir "cmake-$version-windows-x86_64" # 実際の展開先
$zipPath = Join-Path $env:TEMP "cmake-$version.zip"                   # 一時ダウンロード先

# 管理者権限のチェック
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Error "環境変数を書き換えるため、このスクリプトは【管理者権限】で実行する必要があります。batファイルを管理者として開き直してください。"
    exit
}

# 2. 現在利用可能なCMakeのバージョンを確認
$requiredVersion = [Version]$version
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
$installedVersion = $null
$useBinPath = $null
$needInstall = $true

if ($cmakeCommand) {
    try {
        $installedVersionText = (& $cmakeCommand.Source --version | Select-Object -First 1) -replace '^cmake version ', ''
        $installedVersion = [Version]$installedVersionText

        if ($installedVersion -ge $requiredVersion) {
            # 4.3.2以上がすでにある場合はインストールしない
            $needInstall = $false
            $useBinPath = Split-Path $cmakeCommand.Source -Parent
            Write-Host "[INFO] CMake $installedVersion がすでにインストールされています。" -ForegroundColor Green
            Write-Host "[INFO] 必要なバージョン $version 以上なので、インストールをスキップします。" -ForegroundColor Green
        }
        else {
            Write-Host "[INFO] CMake $installedVersion が見つかりましたが、$version より古いため更新します。" -ForegroundColor Yellow
        }
    }
    catch {
        Write-Host "[WARNING] CMakeのバージョンを確認できなかったため、$version をインストールします。" -ForegroundColor Yellow
    }
}
else {
    Write-Host "[INFO] CMakeがインストールされていないため、$version をインストールします。" -ForegroundColor Cyan
}

# 3. 必要な場合のみCMake 4.3.2をインストール
if ($needInstall) {
    Write-Host "[INFO] CMake $version をダウンロード中..." -ForegroundColor Cyan
    Write-Host "URL: $url" -ForegroundColor Gray
    Invoke-WebRequest -Uri $url -OutFile $zipPath -UserAgent "Mozilla/5.0"

    # 展開先のフォルダを準備
    if (-not (Test-Path $installParentDir)) {
        New-Item -ItemType Directory -Path $installParentDir | Out-Null
    }
    if (Test-Path $installDir) {
        Remove-Item -Recurse -Force $installDir
    }

    # ZIPを展開
    Write-Host "[INFO] アーカイブを展開中..." -ForegroundColor Cyan
    Expand-Archive -Path $zipPath -DestinationPath $installParentDir -Force

    Remove-Item -Force $zipPath
    Write-Host "[INFO] 展開完了: $installDir" -ForegroundColor Green

    $useBinPath = $binPath = Join-Path $installDir "bin"
}
else {
    $binPath = $useBinPath
}

# 4. 環境変数 (PATH) を更新
Write-Host "[INFO] 環境変数 (PATH) のチェックと設定中..." -ForegroundColor Cyan

$target = [EnvironmentVariableTarget]::Machine
$oldPath = [Environment]::GetEnvironmentVariable("Path", $target)
$pathEntries = @($oldPath -split ';' | Where-Object { $_ -and $_.Trim() })

# CMakeの旧バージョン用PATHを削除する。
# 今回のインストール先(C:\Program Files\CMake)配下のbinを対象にする。
$cmakeParentFullPath = [IO.Path]::GetFullPath($installParentDir).TrimEnd('\')
$cmakeParentPrefix = $cmakeParentFullPath + '\'

$filteredEntries = @($pathEntries | Where-Object {
    $entry = $_.Trim().TrimEnd('\')
    $isOldCMakePath = $entry.StartsWith($cmakeParentPrefix, [StringComparison]::OrdinalIgnoreCase) -and
                      $entry.EndsWith('\bin', [StringComparison]::OrdinalIgnoreCase)
    -not $isOldCMakePath
})

# 4.3.2をインストールした場合は4.3.2を、
# すでに4.3.2以上がある場合はそのCMakeをPATHの先頭にする。
$filteredEntries = @($binPath) + @($filteredEntries | Where-Object {
    $_.Trim().TrimEnd('\') -ne $binPath.TrimEnd('\')
})

$newPath = $filteredEntries -join ';'

if ($newPath -ne $oldPath) {
    # setxではなく.NET APIを使うことでPATHの長さ制限による切り詰めを避ける
    [Environment]::SetEnvironmentVariable("Path", $newPath, $target)
    Write-Host "[SUCCESS] システムの PATH を更新しました。" -ForegroundColor Green
    Write-Host "[INFO] CMake PATH: $binPath" -ForegroundColor Gray
}
else {
    Write-Host "[INFO] PATHはすでに正しく設定されています。" -ForegroundColor Yellow
}

# 現在のPowerShellセッションにも即時反映
$processPathEntries = @($env:Path -split ';' | Where-Object { $_ -and $_.Trim() })
$processFilteredEntries = @($processPathEntries | Where-Object {
    $entry = $_.Trim().TrimEnd('\')
    $isOldCMakePath = $entry.StartsWith($cmakeParentPrefix, [StringComparison]::OrdinalIgnoreCase) -and
                      $entry.EndsWith('\bin', [StringComparison]::OrdinalIgnoreCase)
    -not $isOldCMakePath
})
$env:Path = (@($binPath) + @($processFilteredEntries | Where-Object {
    $_.Trim().TrimEnd('\') -ne $binPath.TrimEnd('\')
}) -join ';')

# 5. 最終確認
Write-Host "`n=== インストール確認 ===" -ForegroundColor Cyan
$finalCmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($finalCmake) {
    & $finalCmake.Source --version
    Write-Host "`n[COMPLETE] すべての工程が正常に完了しました！" -ForegroundColor Green
}
else {
    Write-Host "[WARNING] CMakeはインストールされていますが、PATHの反映を確認できませんでした。" -ForegroundColor Yellow
}

Pause
