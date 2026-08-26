@echo off
setlocal

set DEVKITPRO=D:\Programlar\Devkitpro
set DEVKITPPC=%DEVKITPRO%\devkitPPC
set PATH=%DEVKITPRO%\tools\bin;%DEVKITPPC%\bin;%PATH%

cd /d %~dp0

if not exist "%DEVKITPPC%\wii_rules" (
    echo DEVKITPPC bulunamadi: %DEVKITPPC%
    exit /b 1
)

echo Wii build basliyor...
make clean
if errorlevel 1 exit /b %errorlevel%

make
if errorlevel 1 exit /b %errorlevel%

if exist "%~dp0naneokey_wii.dol" (
    echo Basarili: %~dp0naneokey_wii.dol
) else (
    echo Build bitti ama .dol bulunamadi.
    exit /b 1
)

endlocal
