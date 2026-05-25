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
$null = Require-File $NinjaPath "Ninja executable"
$null = Require-File (Join-Path $mingwRoot "bin\g++.exe") "MinGW g++ compiler"
$null = Require-File (Join-Path $qtRoot "bin\Qt6Core.dll") "Qt runtime"

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot "build_windows.ps1") -QtPath $qtRoot -CMakePath $cmakeExe -MingwPath $mingwRoot -NinjaPath $NinjaPath -BuildDir $BuildDir -BuildType $BuildType
    if ($LASTEXITCODE -ne 0) {
        throw "Build step failed before run."
    }
}

$appExe = Join-Path (Join-Path $repoRoot $BuildDir) "LabGCSApp.exe"
$null = Require-File $appExe "LabGCS application executable"

$env:Path = (Join-Path $qtRoot "bin") + ";" + (Join-Path $mingwRoot "bin") + ";" + (Split-Path -Parent $cmakeExe) + ";" + $env:Path

Write-Host "Starting LabGCS: $appExe"
& $appExe
if ($LASTEXITCODE -ne 0) {
    throw "LabGCSApp exited with code $LASTEXITCODE."
}
