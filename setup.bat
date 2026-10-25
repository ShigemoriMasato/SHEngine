@echo off

cd /d "%~dp0DirectXGame"

powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CmakeChecker.ps1"

echo.
echo ==========================
echo 0 : ALL
echo 1 : ImGui
echo 2 : DirectXTex
echo 3 : Assimp
echo 4 : FreeType
echo - : Shutdown
echo ==========================

set /p BUILD_MODE=Select Number :

if "%BUILD_MODE%"=="0" goto BUILD_ALL
if "%BUILD_MODE%"=="1" goto BUILD_IMGUI
if "%BUILD_MODE%"=="2" goto BUILD_DIRECTXTEX
if "%BUILD_MODE%"=="3" goto BUILD_ASSIMP
if "%BUILD_MODE%"=="4" goto BUILD_FREETYPE
if "%BUILD_MODE%"=="9" goto COPY_HEADER
if "%BUILD_MODE%"=="-" exit

echo Invalid Number
pause
exit /b 1

:BUILD_ALL

call :CLEAN_GENERATED_FILES

echo === only spdlog ready for debug ===
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\spdlog" -DestDir "externals\header\spdlog"

echo === Clone ===
git clone --branch v6.0.5 --depth 1 --recurse-submodules --shallow-submodules https://github.com/assimp/assimp.git "externals/generated/Project/assimp_clone/"
git clone --branch VER-2-14-3 --depth 1 --recurse-submodules --shallow-submodules https://github.com/freetype/freetype.git "externals/generated/Project/freetype_clone/"

echo ==== Create Slution From Library Source ====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CmakeProject.ps1" -SourceDir "externals\generated\Project\assimp_clone" -BuildDir "externals\generated\Project\assimp"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CmakeProject.ps1" -SourceDir "externals\generated\Project\freetype_clone" -BuildDir "externals\generated\Project\freetype"

echo ===== Create .lib =====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\src\DirectXTex\DirectXTex_Desktop_2022_Win10.sln" -Platform x64
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\src\ForImGui\ForImGui.slnx" -Platform x64
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\generated\Project\freetype\freetype.slnx" -Platform x64
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\generated\Project\assimp\assimp.slnx" -Platform x64

goto END


:BUILD_IMGUI

powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\src\ForImGui\ForImGui.slnx" -Platform x64

goto END


:BUILD_DIRECTXTEX

powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\src\DirectXTex\DirectXTex_Desktop_2022_Win10.sln" -Platform x64

goto END


:BUILD_ASSIMP

call :CLEAN_ASSIMP_FILES

git clone --branch v6.0.5 --depth 1 --recurse-submodules --shallow-submodules https://github.com/assimp/assimp.git "externals/generated/Project/assimp_clone/"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CmakeProject.ps1" -SourceDir "externals\generated\Project\assimp_clone" -BuildDir "externals\generated\Project\assimp"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\generated\Project\assimp\assimp.slnx" -Platform x64

goto END


:BUILD_FREETYPE

call :CLEAN_FREETYPE_FILES

git clone --branch VER-2-14-3 --depth 1 --recurse-submodules --shallow-submodules https://github.com/freetype/freetype.git "externals/generated/Project/freetype_clone/"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CmakeProject.ps1" -SourceDir "externals\generated\Project\freetype_clone" -BuildDir "externals\generated\Project\freetype"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\BuildExternals.ps1" -ProjectPath "externals\generated\Project\freetype\freetype.slnx" -Platform x64

goto END


:COPY_HEADER

goto END



:END

echo ===== Copy Header =====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\DirectXTex\DirectXTex" -DestDir "externals\header\DirectXTex"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\ForImGui\ImGui" -DestDir "externals\header\imgui"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\src\nlohmann" -DestDir "externals\header\nlohmann"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\generated\Project\freetype_clone\include" -DestDir "externals\header"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\generated\Project\assimp_clone\include\assimp" -DestDir "externals\header\assimp"
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CopyHeader.ps1" -SourceDir "externals\generated\Project\assimp\include\assimp" -DestDir "externals\header\assimp"

echo ===== Create Development .lib =====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\CreateDev.ps1"

echo ===== Filter Adjustment =====
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts\FilterAdjust.ps1"

call :CLEAN_GENERATED_FILES

echo ===== Completed =====
pause



:CLEAN_GENERATED_FILES
echo === Clean generated external files ===
for %%D in ("externals\generated\Project\assimp_clone" "externals\generated\Project\assimp" "externals\generated\Project\freetype_clone" "externals\generated\Project\freetype" "externals\generated\else") do (
    if exist "%%~D" (
        rmdir /s /q "%%~D"
        echo Deleted %%~D
    )
)
exit /b 0
