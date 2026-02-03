@echo off
setlocal EnableDelayedExpansion

set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%\.."
set PROJECT_ROOT=%CD%

set INPUT_EXE=%~1
set OUTPUT_HEADER=%~2
set TARGET_NAME=%~3

if "%~3"=="" set TARGET_NAME=target

set INPUT_PATH=%INPUT_EXE%
if not "%INPUT_PATH:~1,1%"==":" (
    set INPUT_PATH=%PROJECT_ROOT%\build\%INPUT_PATH%
)

if not "%OUTPUT_HEADER:~1,1%"==":" (
    set OUTPUT_PATH=%PROJECT_ROOT%\build\%OUTPUT_HEADER%
) else (
    set OUTPUT_PATH=%OUTPUT_HEADER%
)

set SIGTOOL=%PROJECT_ROOT%\build\tools\sigtool\sigtool.exe

if not exist "%SIGTOOL%" (
    echo [ERROR] sigtool.exe not found at: %SIGTOOL%
    echo Please build the project first.
    exit /b 1
)

if not exist "%INPUT_PATH%" (
    echo [ERROR] Input file not found: %INPUT_PATH%
    exit /b 1
)

"%SIGTOOL%" "%INPUT_PATH%" "%OUTPUT_PATH%" "%TARGET_NAME%"
exit /b %ERRORLEVEL%
