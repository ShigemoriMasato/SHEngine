@echo off
setlocal
set "REPO_ROOT=%~dp0.."
set "SUBMIT_DIR=%~dp0ForSubmit"

rmdir /s /q "ForSubmit"

pause

echo [1/4] Release ビルドと実行ファイルのコピー...
powershell.exe -ExecutionPolicy Bypass -NoProfile -File "%~dp0src\BuildForSubmit.ps1"
if errorlevel 1 goto :error

echo [2/4] Assets をコピー...
if not exist "%SUBMIT_DIR%\Assets" mkdir "%SUBMIT_DIR%\Assets"
xcopy "%REPO_ROOT%\DirectXGame\Assets\*" "%SUBMIT_DIR%\Assets\" /E /I /Y /Q
if errorlevel 1 goto :error

echo [3/4] Compiled をコピー...
if not exist "%SUBMIT_DIR%\Compiled" mkdir "%SUBMIT_DIR%\Compiled"
xcopy "%REPO_ROOT%\DirectXGame\Compiled\*" "%SUBMIT_DIR%\Compiled\" /E /I /Y /Q
if errorlevel 1 goto :error

echo [4/4] Assets 内のテクスチャ画像を削除...
powershell.exe -NoProfile -Command "$assetPath = '%SUBMIT_DIR%\Assets'; $extensions = @('.png','.jpg','.jpeg','.bmp','.tga','.tif','.tiff','.dds','.hdr','.exr'); Get-ChildItem -LiteralPath $assetPath -File -Recurse | Where-Object { $extensions -contains $_.Extension.ToLowerInvariant() } | Remove-Item -Force"
if errorlevel 1 goto :error

echo 提出用ファイルの準備が完了しました: "%SUBMIT_DIR%"
exit /b 0

:error
echo エラーが発生したため、提出用ファイルの準備を中断しました。
exit /b 1
