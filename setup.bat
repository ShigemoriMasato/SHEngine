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

rem .h .hppの再配置
powershell -ExecutionPolicy Bypass -NoProfile ^
  -File "Scripts/CopyHeader.ps1" ^
  -SourceDir "externals/src/DirectXTex/DirectXTex" ^
  -DescDir "externals/DirectXTex"

rem filterを作成
powershell -ExecutionPolicy Bypass -NoProfile -File "Scripts/FilterAdjust.ps1"