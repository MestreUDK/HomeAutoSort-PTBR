$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

New-Item -ItemType Directory -Force -Path "lib", "extern", "extern/SkyPrompt" | Out-Null

if (-not (Test-Path "lib/commonlibsse-ng/xmake.lua")) {
    Write-Host "Cloning CommonLibSSE-NG v9.1.0..."
    git clone --depth 1 --branch v9.1.0 --recurse-submodules --shallow-submodules `
        https://github.com/alandtse/CommonLibSSE-NG.git lib/commonlibsse-ng
}

Write-Host "Downloading SKSE Menu Framework API header..."
Invoke-WebRequest `
    -Uri "https://raw.githubusercontent.com/QTR-Modding/SKSE-Menu-Framework-3-API/main/SKSEMenuFramework.h" `
    -OutFile "extern/SKSEMenuFramework.h"

Write-Host "Downloading SkyPrompt API header..."
Invoke-WebRequest `
    -Uri "https://raw.githubusercontent.com/QTR-Modding/SkyPromptAPI/main/include/SkyPrompt/API.hpp" `
    -OutFile "extern/SkyPrompt/API.hpp"

Write-Host "Dependencies ready."
