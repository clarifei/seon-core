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
    exit /b 1
)

:vs_loaded
where cl >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Failed to load Visual Studio environment
    exit /b 1
)

set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%\.."
set PROJECT_ROOT=%CD%

set BUILD_DIR=%PROJECT_ROOT%\build
set OUTPUT_DIR=%PROJECT_ROOT%\output
set MESON_CMD=python -m mesonbuild.mesonmain
set NINJA_CMD=python -m ninja

if "%1"=="debug" (
    set BUILD_TYPE=debug
) else (
    set BUILD_TYPE=release
)

echo SEON Core Build System
echo Build Type: %BUILD_TYPE%
echo.

where python >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not found in PATH
    exit /b 1
)

if not exist "%BUILD_DIR%\build.ninja" (
    echo [ERROR] Build directory not configured
    echo Please run setup first:
    echo   scripts\setup.bat
    echo   scripts\setup.bat debug
    exit /b 1
)

echo [1/3] Configuring...
%MESON_CMD% configure "%BUILD_DIR%" --buildtype=%BUILD_TYPE%
if errorlevel 1 (
    echo [ERROR] Meson configure failed
    exit /b 1
)

echo.
echo [2/3] Building...
%ninja_cmd% -C "%BUILD_DIR%"
if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)

echo.
echo [3/3] Copying outputs to output directory...
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

REM Copy all relevant files to output directory
copy /y "%BUILD_DIR%\seon.exe" "%OUTPUT_DIR%\" >nul 2>&1
copy /y "%BUILD_DIR%\seon.dll" "%OUTPUT_DIR%\" >nul 2>&1
copy /y "%BUILD_DIR%\signature_data.h" "%OUTPUT_DIR%\" >nul 2>&1
copy /y "%BUILD_DIR%\seon.lib" "%OUTPUT_DIR%\" >nul 2>&1
if exist "%BUILD_DIR%\tools\sigtool\sigtool.exe" (
    copy /y "%BUILD_DIR%\tools\sigtool\sigtool.exe" "%OUTPUT_DIR%\" >nul 2>&1
)

echo.
echo Build completed successfully
echo.
echo Output files in: %OUTPUT_DIR%
dir /b "%OUTPUT_DIR%\" 2>nul
echo.

endlocal
