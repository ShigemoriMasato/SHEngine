@echo off

cd /d "%~dp0..\DirectXGame"

powershell -ExecutionPolicy Bypass -NoProfile -File "..\Scripts\src\CMakeIntro.ps1"