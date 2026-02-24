param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Args
)

$repoRoot = Split-Path -Parent $PSScriptRoot
$venvPython = Join-Path $repoRoot ".venv-kicad-mcp\Scripts\python.exe"

if (-not (Test-Path $venvPython)) {
    Write-Error "Missing virtualenv interpreter: $venvPython"
    exit 1
}

& $venvPython -m kicad_mcp @Args
$exitCode = $LASTEXITCODE
exit $exitCode
