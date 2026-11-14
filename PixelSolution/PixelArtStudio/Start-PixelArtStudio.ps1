param([string]$RequestFile, [string]$ResultFile, [switch]$Generate)
$ErrorActionPreference = 'Stop'
$exe = Join-Path $PSScriptRoot '..\bin\Release\PixelArtStudio.exe'
if (-not (Test-Path $exe)) {
    dotnet build (Join-Path $PSScriptRoot 'PixelArtStudio.csproj') -c Release --nologo
    if ($LASTEXITCODE -ne 0) { throw 'Pixel Art Studio build failed.' }
}
$arguments = @()
if ($RequestFile) { $arguments += '--request'; $arguments += ('"' + [IO.Path]::GetFullPath($RequestFile) + '"') }
if ($ResultFile) { $arguments += '--result'; $arguments += ('"' + [IO.Path]::GetFullPath($ResultFile) + '"') }
if ($Generate) { $arguments += '--generate' }
if ($arguments.Count) { Start-Process -FilePath $exe -ArgumentList $arguments } else { Start-Process -FilePath $exe }
