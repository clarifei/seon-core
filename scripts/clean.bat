@echo off
setlocal

set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%\.."
set PROJECT_ROOT=%CD%

set MESON_CMD=python -m mesonbuild.mesonmain

echo Cleaning build artifacts...
echo Project Root: %PROJECT_ROOT%
echo.

if exist "%PROJECT_ROOT%\build" (
    rmdir /s /q "%PROJECT_ROOT%\build"
    echo [OK] Build directory removed.
) else (
    echo [INFO] No build directory found.
)

if exist "%PROJECT_ROOT%\output" (
    rmdir /s /q "%PROJECT_ROOT%\output"
    echo [OK] Output directory removed.
) else (
    echo [INFO] No output directory found.
)

echo.
echo Clean complete.

endlocal
