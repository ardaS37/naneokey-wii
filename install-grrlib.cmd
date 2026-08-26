@echo off
setlocal

set DEVKITPRO=D:\Programlar\Devkitpro
set MSYS_BASH=%DEVKITPRO%\msys2\usr\bin\bash.exe
set GRRLIB_DIR=%~dp0external\GRRLIB

if not exist "%MSYS_BASH%" (
    echo DevkitPro MSYS bash bulunamadi:
    echo %MSYS_BASH%
    exit /b 1
)

if not exist "%~dp0external" mkdir "%~dp0external"

set "SCRIPT=set -e; export DEVKITPRO='/d/Programlar/Devkitpro'; export DEVKITPPC=\"$DEVKITPRO/devkitPPC\"; export PATH=\"$DEVKITPRO/msys2/usr/bin:$DEVKITPRO/devkitPPC/bin:$DEVKITPRO/tools/bin:$PATH\"; pacman --noconfirm -Sy; pacman --noconfirm --needed -S git make mingw-w64-x86_64-pkg-config libfat-ogc ppc-libpng ppc-freetype ppc-libjpeg-turbo; if [ ! -d '/c/Users/ardas/Documents/okey/wii/external/GRRLIB/.git' ]; then git clone https://github.com/GRRLIB/GRRLIB.git '/c/Users/ardas/Documents/okey/wii/external/GRRLIB'; else git -C '/c/Users/ardas/Documents/okey/wii/external/GRRLIB' pull --ff-only; fi; make -C '/c/Users/ardas/Documents/okey/wii/external/GRRLIB' clean all install"

echo GRRLIB kurulumu basliyor...
"%MSYS_BASH%" -lc "%SCRIPT%"
if errorlevel 1 exit /b %errorlevel%

echo.
echo Basarili. GRRLIB burada:
echo %GRRLIB_DIR%
echo.
echo Sonraki adim: build-wii.cmd

endlocal
