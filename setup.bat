@echo off

cd DirectXGame

rem .libの作成
powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/BuildExternals.ps1" ^
  -ProjectPath "externals/src/DirectXTex/DirectXTex_Desktop_2022_Win10.sln" ^
  -Platform x64

powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/BuildExternals.ps1" ^
  -ProjectPath "externals/src/ForImGui/ForImGui.slnx" ^
  -Platform x64

powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/BuildExternals.ps1" ^
  -ProjectPath "externals/src/freetype/freetype.slnx" ^
  -Platform x64

powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/BuildExternals.ps1" ^
  -ProjectPath "externals/src/assimp/assimp.slnx" ^
  -Platform x64

rem .h .hppの再配置
powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/CopyHeader.ps1" ^
  -SourceDir "externals/src/DirectXTex/DirectXTex" ^
  -DestDir "externals/header/DirectXTex"

powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/CopyHeader.ps1" ^
  -SourceDir "externals/src/ForImGui/ImGui" ^
  -DestDir "externals/header/imgui"

powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/CopyHeader.ps1" ^
  -SourceDir "externals/src/spdlog" ^
  -DestDir "externals/header/spdlog"

powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/CopyHeader.ps1" ^
  -SourceDir "externals/src/nlohmann" ^
  -DestDir "externals/header/nlohmann"

powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/CopyHeader.ps1" ^
  -SourceDir "externals/src/freetype-VER-2-14-3/include" ^
  -DestDir "externals/header/"

powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/CopyHeader.ps1" ^
  -SourceDir "externals/src/assimp-6.0.5/include/assimp" ^
  -DestDir "externals/header/assimp"

rem Release用の.libをDevelopment用にコピーして使用できるようにする
powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/CreateDev.ps1" ^

rem filterを作成
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts/FilterAdjust.ps1"