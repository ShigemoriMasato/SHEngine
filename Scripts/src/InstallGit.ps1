# 文字コードをUTF-8に設定
$OutputEncoding = [Console]::OutputEncoding = [System.Text.Encoding]::UTF8

Write-Host "=== Git 自動セットアップスクリプト ===" -ForegroundColor Cyan

# 1. 既にGitがインストールされているか確認
if (Get-Command git -ErrorAction SilentlyContinue) {
    Write-Host "Gitは既にインストールされています！" -ForegroundColor Green
    git --version
    Write-Host "そのままPowerShellでgitコマンドをご利用いただけます。" -ForegroundColor Yellow
    exit
}

# 2. wingetが利用可能かチェック
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
    Write-Host "エラー: パッケージ管理ツール (winget) が見つかりません。" -ForegroundColor Red
    Write-Host "Microsoft Storeから「アプリインストーラー」を更新してください。" -ForegroundColor Yellow
    exit
}

Write-Host "Gitをダウンロード・インストールしています... (しばらくお待ちください)" -ForegroundColor Yellow

# 3. wingetを使ってGit for Windowsをインストール
winget install --id Git.Git -e --source winget --accept-source-agreements --accept-package-agreements

# 4. インストール後のパス再読み込み（現在のセッション用）
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# 5. 完了確認
if (Get-Command git -ErrorAction SilentlyContinue) {
    Write-Host "----------------------------------------" -ForegroundColor Green
    Write-Host "Gitのインストールが正常に完了しました！" -ForegroundColor Green
    git --version
    Write-Host "----------------------------------------" -ForegroundColor Green
} else {
    Write-Host "インストールは完了しましたが、環境変数の反映に時間がかかっている可能性があります。" -ForegroundColor Yellow
    Write-Host "一度PowerShellをすべて閉じてから開き直してください。" -ForegroundColor Yellow
}
