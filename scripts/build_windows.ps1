param(
    [string]$QtPath = $env:GCS_QT_PATH,
    [string]$CMakePath = $env:GCS_CMAKE_PATH,
    [string]$MingwPath = $env:GCS_MINGW_PATH,
    [string]$NinjaPath = $env:GCS_NINJA_PATH,
    [string]$BuildDir = "build",
    [string]$BuildType = "Debug",
    [switch]$CleanConfigure
)

$ErrorActionPreference = "Stop"

function Resolve-RepoRoot {
    return (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
}

function Use-DefaultIfEmpty {
    param([string]$Value, [string]$Default)
    if ([string]::IsNullOrWhiteSpace($Value)) { return $Default }
    return $Value
}

function Require-File {
    param([string]$Path, [string]$Description)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Description was not found at '$Path'. Pass the matching parameter or set the environment variable documented in README.md."
    }
    return (Resolve-Path -LiteralPath $Path).Path
}

function Require-Directory {
    param([string]$Path, [string]$Description)
    if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
        throw "$Description was not found at '$Path'. Pass the matching parameter or set the environment variable documented in README.md."
    }
    return (Resolve-Path -LiteralPath $Path).Path
}

function Convert-ToCMakePath {
    param([string]$Path)
    return $Path.Replace("\", "/")
}

$repoRoot = Resolve-RepoRoot
$QtPath = Use-DefaultIfEmpty $QtPath "C:\QtOnline\6.11.1\mingw_64"
$CMakePath = Use-DefaultIfEmpty $CMakePath "C:\QtOnline\Tools\CMake_64\bin\cmake.exe"
$MingwPath = Use-DefaultIfEmpty $MingwPath "C:\QtOnline\Tools\mingw1310_64"
$NinjaPath = Use-DefaultIfEmpty $NinjaPath "C:\QtOnline\Tools\Ninja\ninja.exe"

$qtRoot = Require-Directory $QtPath "Qt path"
$mingwRoot = Require-Directory $MingwPath "MinGW path"
$cmakeExe = Require-File $CMakePath "CMake executable"
$ninjaExe = Require-File $NinjaPath "Ninja executable"
$compilerExe = Require-File (Join-Path $mingwRoot "bin\g++.exe") "MinGW g++ compiler"
$null = Require-File (Join-Path $qtRoot "bin\Qt6Core.dll") "Qt runtime"

$buildPath = Join-Path $repoRoot $BuildDir

if ($CleanConfigure -and (Test-Path -LiteralPath $buildPath)) {
    Remove-Item -LiteralPath $buildPath -Recurse -Force
}

$env:Path = (Join-Path $qtRoot "bin") + ";" + (Join-Path $mingwRoot "bin") + ";" + (Split-Path -Parent $cmakeExe) + ";" + (Split-Path -Parent $ninjaExe) + ";" + $env:Path

Write-Host "Configuring LabGCS"
Write-Host "  Source: $repoRoot"
Write-Host "  Build:  $buildPath"
Write-Host "  Qt:     $qtRoot"

& $cmakeExe -S $repoRoot -B $buildPath -G Ninja `
    "-DCMAKE_BUILD_TYPE=$BuildType" `
    "-DCMAKE_PREFIX_PATH=$(Convert-ToCMakePath $qtRoot)" `
    "-DCMAKE_CXX_COMPILER=$(Convert-ToCMakePath $compilerExe)" `
    "-DCMAKE_MAKE_PROGRAM=$(Convert-ToCMakePath $ninjaExe)"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed with exit code $LASTEXITCODE."
}

Write-Host "Building LabGCS"
& $cmakeExe --build $buildPath

if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE."
}

Write-Host "Build completed: $(Join-Path $buildPath 'LabGCSApp.exe')"
