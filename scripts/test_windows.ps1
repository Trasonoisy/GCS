param(
    [string]$QtPath = $env:GCS_QT_PATH,
    [string]$CMakePath = $env:GCS_CMAKE_PATH,
    [string]$MingwPath = $env:GCS_MINGW_PATH,
    [string]$NinjaPath = $env:GCS_NINJA_PATH,
    [string]$BuildDir = "build",
    [string]$BuildType = "Debug",
    [switch]$SkipBuild
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

$repoRoot = Resolve-RepoRoot
$QtPath = Use-DefaultIfEmpty $QtPath "C:\QtOnline\6.11.1\mingw_64"
$CMakePath = Use-DefaultIfEmpty $CMakePath "C:\QtOnline\Tools\CMake_64\bin\cmake.exe"
$MingwPath = Use-DefaultIfEmpty $MingwPath "C:\QtOnline\Tools\mingw1310_64"
$NinjaPath = Use-DefaultIfEmpty $NinjaPath "C:\QtOnline\Tools\Ninja\ninja.exe"

$qtRoot = Require-Directory $QtPath "Qt path"
$mingwRoot = Require-Directory $MingwPath "MinGW path"
$cmakeExe = Require-File $CMakePath "CMake executable"
$ctestExe = Require-File (Join-Path (Split-Path -Parent $cmakeExe) "ctest.exe") "CTest executable"
$null = Require-File $NinjaPath "Ninja executable"
$null = Require-File (Join-Path $mingwRoot "bin\g++.exe") "MinGW g++ compiler"
$null = Require-File (Join-Path $qtRoot "bin\Qt6Core.dll") "Qt runtime"

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot "build_windows.ps1") -QtPath $qtRoot -CMakePath $cmakeExe -MingwPath $mingwRoot -NinjaPath $NinjaPath -BuildDir $BuildDir -BuildType $BuildType
    if ($LASTEXITCODE -ne 0) {
        throw "Build step failed before tests."
    }
}

$buildPath = Join-Path $repoRoot $BuildDir
if (-not (Test-Path -LiteralPath (Join-Path $buildPath "CTestTestfile.cmake") -PathType Leaf)) {
    throw "CTest metadata was not found in '$buildPath'. Run scripts\build_windows.ps1 first or omit -SkipBuild."
}

$env:Path = (Join-Path $qtRoot "bin") + ";" + (Join-Path $mingwRoot "bin") + ";" + (Split-Path -Parent $cmakeExe) + ";" + $env:Path

Write-Host "Running LabGCS tests"
& $cmakeExe --build $buildPath
if ($LASTEXITCODE -ne 0) {
    throw "Build failed before tests with exit code $LASTEXITCODE."
}

& $ctestExe --test-dir $buildPath --output-on-failure
if ($LASTEXITCODE -ne 0) {
    throw "Tests failed with exit code $LASTEXITCODE."
}

Write-Host "All tests passed."
