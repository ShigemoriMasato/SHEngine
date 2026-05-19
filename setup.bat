@echo off

cd /d "%~dp0DirectXGame"

powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CmakeChecker.ps1"

pause

echo === Init Directory ===
set "GENERATE_DIR=externals/generated/"
if exist "%GENERATE_DIR%" (
 rmdir /s /q "%GENERATE_DIR%"
)

echo ==== Create Slution From Library Source ====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CmakeProject.ps1" -SourceDir "externals\src\assimp-6.0.5" -BuildDir "externals\generated\Project\assimp"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CmakeProject.ps1" -SourceDir "externals\src\freetype-VER-2-14-3" -BuildDir "externals\generated\Project\freetype"


echo ===== Create .lib =====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\src\DirectXTex\DirectXTex_Desktop_2022_Win10.sln" -Platform x64
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\src\ForImGui\ForImGui.slnx" -Platform x64
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\generated\Project\freetype\freetype.slnx" -Platform x64
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\generated\Project\assimp\assimp.slnx" -Platform x64

echo ===== Copy Header =====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\DirectXTex\DirectXTex" -DestDir "externals\header\DirectXTex"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\ForImGui\ImGui" -DestDir "externals\header\imgui"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\spdlog" -DestDir "externals\header\spdlog"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\nlohmann" -DestDir "externals\header\nlohmann"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\freetype-VER-2-14-3\include" -DestDir "externals\header"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\assimp-6.0.5\include\assimp" -DestDir "externals\header\assimp"

echo ===== Create Development .lib =====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CreateDev.ps1"

echo ===== Filter Adjustment =====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\FilterAdjust.ps1"

echo ===== Completed =====
pause