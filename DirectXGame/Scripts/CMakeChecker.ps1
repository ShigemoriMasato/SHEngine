# エラー発生時もスクリプトがクラッシュしないように設定
$ErrorActionPreference = "SilentlyContinue"

# 基準となるバージョンを定義
$TargetVersionStr = "4.3.2"
$TargetVersion = [version]$TargetVersionStr

Write-Host "=== CMake バージョンチェック ===" -ForegroundColor Cyan

# 1. 現在のシステムで 'cmake' コマンドが利用可能かチェック
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue

if ($cmakeCommand) {
    # 2. cmake --version の出力からバージョン番号（例: 3.25.1）を抽出
    $cmakeVersionOutput = (cmake --version | Select-Object -First 1)
    if ($cmakeVersionOutput -match 'version\s+([\d\.]+)') {
        $CurrentVersionStr = $Matches[1]
        $CurrentVersion = [version]$CurrentVersionStr
        
        Write-Host "現在のバージョン: $CurrentVersionStr" -ForegroundColor Gray
        Write-Host "必要なバージョン: $TargetVersionStr 以上`n" -ForegroundColor Gray

        # 3. バージョン比較 (現在 >= ターゲット)
        if ($CurrentVersion -ge $TargetVersion) {
            Write-Host "[OK] 最新、または十分なバージョンの CMake がインストールされています。" -ForegroundColor Green
            Write-Host "通常終了します。" -ForegroundColor Green
            exit 0
        } else {
            Write-Host "[WARNING] CMake のバージョンが古いです。" -ForegroundColor Yellow
        }
    } else {
        Write-Host "[ERROR] CMake のバージョン情報を正しく解析できませんでした。" -ForegroundColor Red
    }
} else {
    Write-Host "[NOT FOUND] CMake がシステムにインストールされていないか、PATHが通っていません。" -ForegroundColor Red
}

# --------------------------------------------------------------------
# 4. 【古い場合 / 未インストールの場合】の共通リアクション
# --------------------------------------------------------------------
Write-Host "`n--------------------------------------------------" -ForegroundColor DarkCyan
Write-Host "【重要】CMake $TargetVersionStr 以上の導入が必要です。" -ForegroundColor Yellow
Write-Host "お手数ですが、『CMakeRegister.bat』を起動してインストールを行ってください。" -ForegroundColor Cyan
Write-Host "--------------------------------------------------`n" -ForegroundColor DarkCyan

# キー入力を待機（pause の再現）
Write-Host "続行するには、キーボードの任意のキーを押してください . . . " -NoNewline
[void]$Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

# セッションを閉じる（呼び出し元のウィンドウごと落とす）
Stop-Process -Id $PID