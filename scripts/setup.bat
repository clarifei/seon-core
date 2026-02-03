@echo off
setlocal EnableDelayedExpansion

REM Try to load VS environment if not already loaded
where cl >nul 2>&1
if errorlevel 1 (
    echo [INFO] Visual Studio environment not detected, attempting to load...
    
    set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist !VSWHERE! (
        for /f "tokens=*" %%i in ('!VSWHERE! -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do (
            set VS_PATH=%%i
            if exist "%%i\VC\Auxiliary\Build\vcvars64.bat" (
                call "%%i\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
                goto :vs_loaded
            )
        )
    )
    
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
        goto :vs_loaded
    )
    
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        call "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
        goto :vs_loaded
    )
    
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
        call "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
        goto :vs_loaded
    )
    
    echo [ERROR] Could not find Visual Studio vcvars64.bat
    echo Please run this script from "Developer Command Prompt for VS 2022"
    echo Or install Visual Studio with C++ workload
    exit /b 1
)

:vs_loaded
where cl >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Failed to load Visual Studio environment
    exit /b 1
)

echo [OK] Visual Studio environment loaded

set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%\.."
set PROJECT_ROOT=%CD%

set BUILD_DIR=%PROJECT_ROOT%\build
set OUTPUT_DIR=%PROJECT_ROOT%\output
set BUILD_TYPE=release
set MESON_CMD=python -m mesonbuild.mesonmain

if "%1"=="debug" (
    set BUILD_TYPE=debug
)

echo.
echo SEON Core Setup
echo Project Root: %PROJECT_ROOT%
echo Build Type: %BUILD_TYPE%
echo.

where python >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not found in PATH
    exit /b 1
)

%MESON_CMD% --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Meson not found
    echo Please install: pip install meson ninja
    exit /b 1
)

echo [1/3] Stopping any running processes...
taskkill /f /im seon.exe 2>nul >nul
taskkill /f /im sigtool.exe 2>nul >nul
ping -n 2 127.0.0.1 >nul

echo [2/3] Cleaning old directories...
if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%" 2>nul
    if exist "%BUILD_DIR%" (
        echo [WARNING] Could not remove build directory completely, retrying...
        ping -n 2 127.0.0.1 >nul
        rmdir /s /q "%BUILD_DIR%" 2>nul
    )
)
if exist "%OUTPUT_DIR%" (
    rmdir /s /q "%OUTPUT_DIR%" 2>nul
)

echo [3/3] Running meson setup...
%MESON_CMD% setup "%BUILD_DIR%" --buildtype=%BUILD_TYPE%
if errorlevel 1 (
    echo [ERROR] Meson setup failed
    exit /b 1
)

echo.
echo Setup completed successfully
echo Build directory: %BUILD_DIR%
echo Output directory: %OUTPUT_DIR% (will be created after build)
echo.
echo Now run: scripts\build.bat

endlocal
