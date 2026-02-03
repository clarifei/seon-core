@echo off
setlocal EnableDelayedExpansion

if "%1"=="debug" (
    set BUILD_TYPE=debug
) else (
    set BUILD_TYPE=release
)

echo SEON Core Rebuild (Setup + Build)
echo Build Type: %BUILD_TYPE%
echo.

set SCRIPT_DIR=%~dp0

call "%SCRIPT_DIR%\setup.bat" %BUILD_TYPE%
if errorlevel 1 (
    echo [ERROR] Setup failed
    exit /b 1
)

call "%SCRIPT_DIR%\build.bat" %BUILD_TYPE%
if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)

echo.
echo Rebuild completed successfully
echo.

endlocal
