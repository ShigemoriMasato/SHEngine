@echo off
setlocal enabledelayedexpansion

rem !!!==================================================================================!!!
rem !!!				PLEASE CHANGE THIS NAME!!!!!!				!!!
rem !!!==================================================================================!!!

set SUBJECT_NAME=SR1

rem !!!==================================================================================!!!
rem !!!				PLEASE CHANGE THIS NAME!!!!!!				!!!
rem !!!==================================================================================!!!

:LOOP
echo.
echo ------------------------------------
set /p INPUT_NUM="Enter 4-digit number: "

REM --- Input Validation ---
REM 1. Empty check
if "%INPUT_NUM%"=="" (
    echo [ERROR] Input is empty. Please enter 4 digits.
    goto LOOP
)

REM 2. Length check (Error if 5th character exists)
if not "%INPUT_NUM:~4,1%"=="" (
    echo [ERROR] Input must be exactly 4 digits.
    goto LOOP
)

REM 3. Numeric check
echo %INPUT_NUM%| findstr /r "^[0-9][0-9][0-9][0-9]$" > nul
if errorlevel 1 (
    echo [ERROR] Input contains non-numeric characters.
    goto LOOP
)

REM --- String Formatting ---
set "AB=%INPUT_NUM:~0,2%"
set "CD=%INPUT_NUM:~2,2%"

set "TAG_NAME=%SUBJECT_NAME%_%AB%_%CD%"

echo.
echo Creating release... [%TAG_NAME%]

REM Execute gh release create
gh release create "%TAG_NAME%" --title "%TAG_NAME%" --notes ""

if %ERRORLEVEL% equ 0 (
    echo [SUCCESS] Released %TAG_NAME% successfully.
) else (
    echo [ERROR] Failed to create release %TAG_NAME%.
)

goto LOOP