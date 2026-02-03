@echo off
setlocal EnableDelayedExpansion

set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%\.."
set PROJECT_ROOT=%CD%

set OUTPUT_DIR=%PROJECT_ROOT%\output
set SEON_EXE=%OUTPUT_DIR%\seon.exe

echo SEON Core Runner
echo.

if not exist "%SEON_EXE%" (
    echo [ERROR] seon.exe not found at: %SEON_EXE%
    echo Please build first:
    echo   scripts\setup.bat
    echo   scripts\build.bat
    exit /b 1
)

echo Running SEON Core...
echo Executable: %SEON_EXE%
echo.

cd /d "%OUTPUT_DIR%"
seon.exe

endlocal
