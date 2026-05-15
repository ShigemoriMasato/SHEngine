param(
    [string]$SourceDir = "C:\src",
    [string]$DestDir   = "C:\dest"
)

$SourceDir = (Resolve-Path $SourceDir).Path
$resolved = Resolve-Path $DestDir -ErrorAction SilentlyContinue

if ($resolved) {
    $DestDir = $resolved.Path
}

if (!(Test-Path $DestDir)) {
    New-Item -ItemType Directory -Path $DestDir | Out-Null
}

Get-ChildItem -Path $SourceDir -Recurse -Include *.h, *.hpp, *.inl -File | ForEach-Object {

    $srcPath = $_.FullName

    $relativePath = $srcPath.Substring($SourceDir.Length).TrimStart('\')

    $destPath = Join-Path $DestDir $relativePath

    $destDirPath = Split-Path $destPath
    if (!(Test-Path $destDirPath)) {
        New-Item -ItemType Directory -Path $destDirPath -Force | Out-Null
    }

    Write-Host $srcPath "->" $destPath
    Copy-Item -Path $srcPath -Destination $destPath -Force
}

Write-Host "Copy completed."