$ErrorActionPreference = "Stop"

$env:DEVKITPRO = "D:\Programlar\Devkitpro"
$env:DEVKITPPC = Join-Path $env:DEVKITPRO "devkitPPC"
$env:Path = "$env:DEVKITPRO\tools\bin;$env:DEVKITPPC\bin;$env:Path"

Set-Location $PSScriptRoot

if (-not (Test-Path (Join-Path $env:DEVKITPPC "wii_rules"))) {
    throw "DEVKITPPC bulunamadi: $env:DEVKITPPC"
}

Write-Host "Wii build basliyor..."
& make clean
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& make
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$dol = Join-Path $PSScriptRoot "naneokey_wii.dol"
if (-not (Test-Path $dol)) {
    throw ".dol uretilmedi."
}

Write-Host "Basarili: $dol"
