@echo off
setlocal
set "REPO_ROOT=%~dp0.."
set "SUBMIT_DIR=%~dp0ForSubmit"

rmdir /s /q "ForSubmit"

echo [1/5] Release ビルドと実行ファイルのコピー...
powershell.exe -ExecutionPolicy Bypass -NoProfile -File "%~dp0src\BuildForSubmit.ps1"
if errorlevel 1 goto :error

rem DirectXTex が OpenMP を使うため、exe は vcomp140.dll に依存する(Windows 標準には入っていない)
echo [2/5] OpenMP ランタイム (vcomp140.dll) をコピー...
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VS_PATH="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -property installationPath`) do set "VS_PATH=%%i"
if not defined VS_PATH (
    echo Visual Studio が見つかりません。
    goto :error
)
rem ビルドに使ったツールセットと同じバージョンの再頒布ファイルを使う
set "REDIST_VERSION="
for /f "usebackq delims=" %%v in ("%VS_PATH%\VC\Auxiliary\Build\Microsoft.VCRedistVersion.default.txt") do set "REDIST_VERSION=%%v"
set "VCOMP_DLL="
for /d %%d in ("%VS_PATH%\VC\Redist\MSVC\%REDIST_VERSION%\x64\Microsoft.VC*.OpenMP") do set "VCOMP_DLL=%%d\vcomp140.dll"
if not defined VCOMP_DLL (
    echo vcomp140.dll が見つかりません。Visual Studio の C++ 再頒布ファイルを確認してください。
    goto :error
)
if not exist "%VCOMP_DLL%" (
    echo vcomp140.dll が見つかりません: "%VCOMP_DLL%"
    goto :error
)
if not exist "%SUBMIT_DIR%" mkdir "%SUBMIT_DIR%"
copy /Y "%VCOMP_DLL%" "%SUBMIT_DIR%\vcomp140.dll" >nul
if errorlevel 1 goto :error

echo [3/5] Assets をコピー...
if not exist "%SUBMIT_DIR%\Assets" mkdir "%SUBMIT_DIR%\Assets"
xcopy "%REPO_ROOT%\DirectXGame\Assets\*" "%SUBMIT_DIR%\Assets\" /E /I /Y /Q
if errorlevel 1 goto :error

echo [4/5] Compiled をコピー...
if not exist "%SUBMIT_DIR%\Compiled" mkdir "%SUBMIT_DIR%\Compiled"
xcopy "%REPO_ROOT%\DirectXGame\Compiled\*" "%SUBMIT_DIR%\Compiled\" /E /I /Y /Q
if errorlevel 1 goto :error

echo [5/5] Assets 内のテクスチャ画像を削除...
powershell.exe -NoProfile -Command "$assetPath = '%SUBMIT_DIR%\Assets'; $extensions = @('.png','.jpg','.jpeg','.bmp','.tga','.tif','.tiff','.dds','.hdr','.exr'); Get-ChildItem -LiteralPath $assetPath -File -Recurse | Where-Object { $extensions -contains $_.Extension.ToLowerInvariant() } | Remove-Item -Force"
if errorlevel 1 goto :error

echo 提出用ファイルの準備が完了しました: "%SUBMIT_DIR%"
exit /b 0

:error
echo エラーが発生したため、提出用ファイルの準備を中断しました。
exit /b 1
