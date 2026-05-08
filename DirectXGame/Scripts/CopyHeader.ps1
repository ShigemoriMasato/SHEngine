param(
    [string]$SourceDir = "C:\src",
    [string]$DestDir   = "C:\dest"
)

# フルパス化（安全のため）
$SourceDir = (Resolve-Path $SourceDir).Path
$resolved = Resolve-Path $DestDir -ErrorAction SilentlyContinue

if ($resolved) {
    $DestDir = $resolved.Path
}

# 出力先フォルダがなければ作成
if (!(Test-Path $DestDir)) {
    New-Item -ItemType Directory -Path $DestDir | Out-Null
}

# .h / .hpp を再帰取得
Get-ChildItem -Path $SourceDir -Recurse -Include *.h, *.hpp -File | ForEach-Object {

    # 元ファイルのフルパス
    $srcPath = $_.FullName

    # SourceDirからの相対パスを作る
    $relativePath = $srcPath.Substring($SourceDir.Length).TrimStart('\')

    # コピー先のフルパス
    $destPath = Join-Path $DestDir $relativePath

    # コピー先のディレクトリ作成
    $destDirPath = Split-Path $destPath
    if (!(Test-Path $destDirPath)) {
        New-Item -ItemType Directory -Path $destDirPath -Force | Out-Null
    }

    # コピー
    Copy-Item -Path $srcPath -Destination $destPath -Force
}

Write-Host "Copy completed."