$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$dll = Get-ChildItem -Path "build" -Recurse -Filter "HomeAutoSort.dll" -File | Select-Object -First 1
if (-not $dll) {
    throw "HomeAutoSort.dll was not found under build/. The compilation did not produce the expected DLL."
}

$artifact = Join-Path $root "artifact"
$plugins = Join-Path $artifact "SKSE/Plugins"
$zip = Join-Path $root "HomeAutoSort_PTBR_2.4_GitHubBuild.zip"

Remove-Item -Recurse -Force $artifact -ErrorAction SilentlyContinue
Remove-Item -Force $zip -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $plugins | Out-Null

Copy-Item $dll.FullName (Join-Path $plugins "HomeAutoSort.dll")
Copy-Item "dist/SKSE/Plugins/HomeAutoSort_Translation.ini" (Join-Path $plugins "HomeAutoSort_Translation.ini")
Copy-Item "COPYING.txt" (Join-Path $artifact "COPYING.txt")
Copy-Item "EXCEPTIONS.md" (Join-Path $artifact "EXCEPTIONS.md")

Compress-Archive -Path "$artifact/*" -DestinationPath $zip -Force

Write-Host "Package created: $zip"
Write-Host "DLL source: $($dll.FullName)"
