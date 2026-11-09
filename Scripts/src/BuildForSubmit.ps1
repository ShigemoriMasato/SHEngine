$ErrorActionPreference = 'Stop'

# スクリプトの場所からリポジトリのルートを取得するため、どの作業ディレクトリからでも実行できます。
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$solutionPath = Join-Path $repoRoot 'SHEngine.sln'
$buildOutput = Join-Path $repoRoot 'Generated\Release'
$submitDirectory = Join-Path $repoRoot 'Scripts\ForSubmit'

New-Item -ItemType Directory -Path $submitDirectory -Force | Out-Null

# SHEngine.sln は CMake の生成物ではないため、CMake ではなく Visual Studio MSBuild でビルドする。
$msbuildCommand = Get-Command msbuild.exe -ErrorAction SilentlyContinue
$msbuildPath = if ($msbuildCommand) { $msbuildCommand.Source } else { $null }
$vswherePath = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$visualStudioPath = $null

if (Test-Path -LiteralPath $vswherePath) {
    $visualStudioPath = & $vswherePath -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
}

if (-not $msbuildPath) {
    if ($visualStudioPath) {
        $msbuildPath = & $vswherePath -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    }
}

if (-not $msbuildPath -or -not (Test-Path -LiteralPath $msbuildPath -PathType Leaf)) {
    throw 'Visual Studio の MSBuild.exe が見つかりません。Visual Studio の C++ ビルドツールを確認してください。'
}

# Visual Studio の開発環境を読み込み、cl.exe と PDB 関連 DLL が同じツールセットになるようにする。
if ($visualStudioPath) {
    $vsDevCmdPath = Join-Path $visualStudioPath 'Common7\Tools\VsDevCmd.bat'
    if (Test-Path -LiteralPath $vsDevCmdPath) {
        $environmentLines = & cmd.exe /d /c "call `"$vsDevCmdPath`" -arch=amd64 -host_arch=amd64 >nul && set"
        if ($LASTEXITCODE -ne 0) {
            throw 'Visual Studio の開発環境 (VsDevCmd.bat) を初期化できませんでした。'
        }

        foreach ($line in $environmentLines) {
            if ($line -match '^([^=]+)=(.*)$') {
                Set-Item -Path "Env:$($Matches[1])" -Value $Matches[2]
            }
        }
    }
}

Write-Host 'SHEngine.sln を Release | x64 でビルドします...'
& $msbuildPath $solutionPath '/t:Build' '/m' '/p:Configuration=Release' '/p:Platform=x64' '/verbosity:minimal'
if ($LASTEXITCODE -ne 0) {
    throw "ビルドに失敗しました (exit code: $LASTEXITCODE)。C1902 が出る場合は Visual Studio の C++ ツールセットと mspdb140.dll / mspdbcore.dll / mspdbsrv.exe のバージョン不一致が原因です。Visual Studio Installer で C++ によるデスクトップ開発を修復してください。"
}

$filesToCopy = @('DirectXGame.exe', 'dxcompiler.dll', 'dxil.dll')
foreach ($fileName in $filesToCopy) {
    $sourcePath = Join-Path $buildOutput $fileName
    if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
        throw "ビルド出力が見つかりません: $sourcePath"
    }
}

foreach ($fileName in $filesToCopy) {
    Copy-Item -LiteralPath (Join-Path $buildOutput $fileName) -Destination $submitDirectory -Force
    Write-Host "コピーしました: $fileName"
}

Write-Host "提出用ファイルを配置しました: $submitDirectory"
