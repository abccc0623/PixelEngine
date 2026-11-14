param([switch]$Generate)
$ErrorActionPreference='Stop'
$exe=Join-Path $PSScriptRoot '..\bin\Release\PixelArtStudio.exe'
$dir=Join-Path $env:LOCALAPPDATA ('PixelArtStudio\headless-tests\'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force $dir | Out-Null
$request=Join-Path $dir 'request.json'
@{prompt='a cute blue slime monster, front view, round body, two black eyes, no shadow';width=64;height=64;steps=20;seed=12345;transparent=$true;outputPath=(Join-Path $dir 'sprite.png')} | ConvertTo-Json | Set-Content $request -Encoding UTF8
function Run-Test($name,$extra,$expected,$expectedStatus,$requestFile=$request) {
    $result=Join-Path $dir ($name+'.json')
    $arguments=@('--mode','background','--request',('"'+$requestFile+'"'),'--result',('"'+$result+'"'))+$extra
    $p=Start-Process $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru
    $window=$false
    if ($name -eq 'cancel') { Start-Sleep -Milliseconds 700; New-Item -ItemType File (Join-Path $dir 'cancel.signal') | Out-Null }
    while (-not $p.HasExited) { $p.Refresh(); if ($p.MainWindowHandle -ne 0) {$window=$true}; Start-Sleep -Milliseconds 100 }
    $p.Refresh()
    $r=Get-Content $result -Raw | ConvertFrom-Json
    if ($p.ExitCode -ne $expected -or $r.status -ne $expectedStatus -or $window) {throw "$name failed: exit=$($p.ExitCode), state=$($r.status), window=$window"}
    "$name passed: exit=$($p.ExitCode), status=$($r.status), window=$window"
}
$bad=Join-Path $dir 'bad-request.json'; '{"prompt":""}' | Set-Content $bad -Encoding UTF8
Run-Test 'invalid' @() 1 'failed' $bad
Run-Test 'cancel' @('--cancel-file',('"'+(Join-Path $dir 'cancel.signal')+'"')) 130 'cancelled'
Run-Test 'timeout' @('--timeout-seconds','1') 124 'failed'
if ($Generate) {Run-Test 'generate' @() 0 'completed'; if (-not (Test-Path (Join-Path $dir 'sprite.png'))) {throw 'PNG missing'}}
"Results: $dir"
