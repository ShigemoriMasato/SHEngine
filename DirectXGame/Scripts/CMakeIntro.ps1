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
    Write-Error "環境変数を書き換えるため、このスクリプトは【管理者権限】で実行する必要があります。PowerShellを管理者として開き直してください。"
    exit
}

# 2. すでに同じバージョンが入っていないかチェック
if (Test-Path (Join-Path $installDir "bin\cmake.exe")) {
    Write-Host "[INFO] CMake $version はすでに $installDir にインストールされています。" -ForegroundColor Green
} else {
    # 3. ダウンロード処理
    Write-Host "[INFO] CMake $version をダウンロード中..." -ForegroundColor Cyan
    Write-Host "URL: $url" -ForegroundColor Gray
    Invoke-WebRequest -Uri $url -OutFile $zipPath -UserAgent "Mozilla/5.0"

    # 4. 展開先のフォルダを綺麗に準備
    if (-not (Test-Path $installParentDir)) {
        New-Item -ItemType Directory -Path $installParentDir | Out-Null
    }
    if (Test-Path $installDir) {
        Remove-Item -Recurse -Force $installDir
    }

    # 5. ZIPの展開
    Write-Host "[INFO] アーカイブをコピー・展開中..." -ForegroundColor Cyan
    Expand-Archive -Path $zipPath -DestinationPath $installParentDir -Force

    # 後片付け（一時フォルダのZIPを削除）
    Remove-Item -Force $zipPath
    Write-Host "[INFO] 展開完了: $installDir" -ForegroundColor Green
}

# 6. 環境変数 (PATH) への追加処理
$binPath = Join-Path $installDir "bin"
Write-Host "[INFO] 環境変数 (PATH) のチェックと設定中..." -ForegroundColor Cyan

# システム全体のPATHを取得
$target = [EnvironmentVariableTarget]::Machine
$oldPath = [Environment]::GetEnvironmentVariable("Path", $target)

# 重複していなければ末尾に追加
if ($oldPath -split ';' -contains $binPath) {
    Write-Host "[INFO] すでに PATH に登録されています。" -ForegroundColor Yellow
} else {
    # 古いバージョンのCMakeのPATHが残っていたら紛らわしいので、一応綺麗にする（任意）
    # ※もし必要なら手動で古いPATHを削ってください
    [Environment]::GetEnvironmentVariable("Path", $target)
    # Windows標準の外部コマンドで、システム環境変数にPATHを強制追加する
    setx /M PATH "$oldPath;$binPath"
    
    # 現在のPowerShellセッションのPATHにも即時反映
    $env:Path += ";$binPath"
    Write-Host "[SUCCESS] システムの PATH に $binPath を追加しました！" -ForegroundColor Green
}

# 7. 最終確認
Write-Host "`n=== インストール確認 ===" -ForegroundColor Cyan
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    cmake --version
    Write-Host "`n[COMPLETE] すべての工程が正常に完了しました！" -ForegroundColor Green
} else {
    Write-Host "[WARNING] インストールはできましたが、PATHの反映にはPowerShellやPCの再起動が必要な場合があります。" -ForegroundColor Yellow
}

Pause