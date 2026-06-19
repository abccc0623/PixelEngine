$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$pythonCandidates = @(
    "C:\Users\abcc0\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe",
    "python",
    "py"
)

$python = $null
foreach ($candidate in $pythonCandidates) {
    try {
        $cmd = Get-Command $candidate -ErrorAction Stop
        $python = $cmd.Source
        break
    } catch {
    }
}

if (-not $python) {
    throw "Python executable was not found."
}

& $python "$PSScriptRoot\server.py" --host 127.0.0.1 --port 8765
