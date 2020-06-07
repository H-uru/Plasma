#!/usr/bin/env pwsh
#Requires –Version 5.0

param([string]$builddir='build')

$devlibs_url = "https://github.com/H-uru/PlasmaPrefix/releases/download/2020.05.01/devlibs.zip"

if (!(Test-Path -PathType Container $builddir)) {
    Write-Host "Creating build folder at $builddir... " -noNewLine
    New-Item -ItemType directory $builddir | Out-Null
    Write-Host "OK" -foregroundColor Green
}
Set-Location $builddir
$path = (Get-Location).Path

if (!(Test-Path -PathType Container devlibs)) {
    # Hide progress output
    $ProgressPreference = 'SilentlyContinue'

    $zippath = Join-Path -Path $path -ChildPath "devlibs.zip"
    $libpath = Join-Path -Path $path -ChildPath "devlibs"

    Write-Host "Downloading development libraries... " -noNewLine
    Invoke-WebRequest $devlibs_url -OutFile $zippath
    Write-Host "OK" -foregroundColor Green

    Write-Host "Extracting development libraries... " -noNewLine
    New-Item -ItemType directory devlibs | Out-Null
    Expand-Archive -Path $zippath -DestinationPath $libpath
    Write-Host "OK" -foregroundColor Green

    # Resume normal progress output
    $ProgressPreference = 'Continue'
}

if (Get-ChildItem Env:PATH | Where-Object {$_.Value -match "CMake"}) {
    Write-Host "Running CMake to configure build system... "
    cmake -DCMAKE_INSTALL_PREFIX=devlibs -DPLASMA_BUILD_TOOLS=OFF -DPLASMA_BUILD_RESOURCE_DAT=OFF -A Win32 -G "Visual Studio 15 2017" ..
} else {
    Write-Host "CMake not found in PATH."
    Write-Host "Please run the CMake installer and select the option to add CMake to your system PATH."
}

if ($Host.Name -eq "ConsoleHost") {
    Write-Host ""
    Write-Host "Press any key to continue..."
    $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyUp") > $null
}
