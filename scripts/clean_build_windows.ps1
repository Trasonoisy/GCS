param(
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$buildPath = Join-Path $repoRoot $BuildDir

if (-not (Test-Path -LiteralPath $buildPath)) {
    Write-Host "Nothing to clean: '$buildPath' does not exist."
    exit 0
}

$resolvedBuildPath = (Resolve-Path -LiteralPath $buildPath).Path
$repoRootWithSlash = $repoRoot.TrimEnd("\") + "\"
if (-not $resolvedBuildPath.StartsWith($repoRootWithSlash, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to remove '$resolvedBuildPath' because it is outside the repository root '$repoRoot'."
}

if ($resolvedBuildPath -eq $repoRoot) {
    throw "Refusing to remove the repository root."
}

Write-Host "Removing build directory: $resolvedBuildPath"
Remove-Item -LiteralPath $resolvedBuildPath -Recurse -Force
Write-Host "Clean completed."
