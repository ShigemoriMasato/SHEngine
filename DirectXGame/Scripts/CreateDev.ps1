Copy-Item -Path "externals/generated/Lib/Release/*" -Destination "externals/generated/Lib/Development" -Recurse -Force

Remove-Item "externals/generated/Lib/Release/ImGui.lib"