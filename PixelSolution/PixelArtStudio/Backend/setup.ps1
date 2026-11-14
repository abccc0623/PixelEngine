
$ErrorActionPreference = 'Stop'
$root = Join-Path $env:LOCALAPPDATA 'PixelArtStudio'
$python = Join-Path $root 'Python311\python.exe'
New-Item -ItemType Directory -Force $root | Out-Null
if (-not (Test-Path $python)) {
    $installer = Join-Path $root 'python-3.11.9-amd64.exe'
    if (-not (Test-Path $installer)) { Invoke-WebRequest 'https://www.python.org/ftp/python/3.11.9/python-3.11.9-amd64.exe' -OutFile $installer }
    $signature = Get-AuthenticodeSignature $installer
    if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'Python Software Foundation') { throw 'Python installer signature is not valid.' }
    $target = Join-Path $root 'Python311'
    $process = Start-Process $installer -ArgumentList @('/quiet', 'InstallAllUsers=0', "TargetDir=`"$target`"", 'PrependPath=0', 'Include_launcher=0', 'Include_test=0', 'Include_doc=0', 'AssociateFiles=0', 'Shortcuts=0') -Wait -PassThru -WindowStyle Hidden
    if ($process.ExitCode -ne 0) { throw "Python installer exited with $($process.ExitCode)." }
}
$venv = Join-Path $root 'venv'
$runtime = Join-Path $venv 'Scripts\python.exe'
if (-not (Test-Path $runtime)) { & $python -m venv $venv; if ($LASTEXITCODE -ne 0) { throw 'venv creation failed.' } }
& $runtime -m pip install --upgrade pip
if ($LASTEXITCODE -ne 0) { throw 'pip setup failed.' }
& $runtime -m pip install torch==2.7.1 torchvision==0.22.1 --index-url https://download.pytorch.org/whl/cu126
if ($LASTEXITCODE -ne 0) { throw 'PyTorch installation failed.' }
& $runtime -m pip install -r (Join-Path $PSScriptRoot 'requirements.txt')
if ($LASTEXITCODE -ne 0) { throw 'Backend dependencies installation failed.' }
& $runtime (Join-Path $PSScriptRoot 'generate.py') --check
if ($LASTEXITCODE -ne 0) { throw 'GPU backend check failed.' }
Write-Output 'Runtime installed. Model weights are downloaded on first generation (approximately 8 GB).'

