@echo off
setlocal

cd /d %~dp0

if not exist "naneokey_wii.dol" (
    echo Once build-wii.cmd calistir.
    exit /b 1
)

if not exist "homebrew\apps\naneokey" mkdir "homebrew\apps\naneokey"

copy /Y "naneokey_wii.dol" "homebrew\apps\naneokey\boot.dol" >nul

if not exist "homebrew\apps\naneokey\icon.png" (
    echo icon.png eksik.
    exit /b 1
)

if not exist "homebrew\apps\naneokey\meta.xml" (
    echo meta.xml eksik.
    exit /b 1
)

echo Hazir:
echo %cd%\homebrew\apps\naneokey
echo.
echo SD karta su klasoru kopyala:
echo apps\naneokey

endlocal
