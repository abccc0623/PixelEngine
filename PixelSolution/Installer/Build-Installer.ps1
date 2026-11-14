param(
    [Parameter(Mandatory = $true)][string]$SourceDir,
    [string]$OutputDir,
    [string]$CompilerPath,
    [switch]$ValidateOnly
)
$ErrorActionPreference = 'Stop'
if (-not $OutputDir) { $OutputDir = Join-Path $PSScriptRoot '..\bin\Installer' }
$source = (Resolve-Path -LiteralPath $SourceDir).Path
$required = @('PixelTool.exe','PixelTool.dll','PixelTool.deps.json','PixelTool.runtimeconfig.json',
    'PixelEngine.dll','PixelGraphics.dll','PixelMeta.dll','lua51.dll','Client.exe',
    'LSP\bin\lua-language-server.exe')
foreach ($relative in $required) {
    if (-not (Test-Path -LiteralPath (Join-Path $source $relative) -PathType Leaf)) {
        throw "Installer input missing: $relative ($source)"
    }
}
if (-not $CompilerPath) {
    $candidates = @(
        "$env:LOCALAPPDATA\Programs\Inno Setup 7\ISCC.exe",
        "$env:ProgramFiles\Inno Setup 7\ISCC.exe",
        "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
    )
    $CompilerPath = $candidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
}
if (-not $CompilerPath -or -not (Test-Path -LiteralPath $CompilerPath -PathType Leaf)) {
    throw 'Inno Setup compiler not found. Install Inno Setup or set InnoSetupCompilerPath in MSBuild.'
}
$version = [Diagnostics.FileVersionInfo]::GetVersionInfo((Join-Path $source 'PixelTool.dll')).FileVersion
if (-not $version) { $version = '1.0.0.0' }
if ($ValidateOnly) {
    Write-Output "PASS: installer inputs and compiler found. Version=$version; Source=$source"
    return
}
$output = [IO.Path]::GetFullPath($OutputDir)
$publish = Join-Path $output ('Payload-' + [guid]::NewGuid().ToString('N'))
# A fresh directory prevents old framework or application files from leaking into the installer.
$project = Join-Path $PSScriptRoot '..\PixelTool\PixelTool.csproj'
& dotnet publish $project --configuration Release --runtime win-x64 --self-contained true --output $publish '-p:BuildInstaller=false' '-p:InstallerPublish=true'
if ($LASTEXITCODE -ne 0) { throw "Self-contained publish failed with exit code $LASTEXITCODE" }
foreach ($relative in @('PixelEngine.dll','PixelGraphics.dll','PixelMeta.dll','lua51.dll','Client.exe')) {
    Copy-Item -LiteralPath (Join-Path $source $relative) -Destination (Join-Path $publish $relative) -Force
}
$artProject = Join-Path $PSScriptRoot '..\PixelArtStudio\PixelArtStudio.csproj'
& dotnet publish $artProject --configuration Release --runtime win-x64 --self-contained true --output (Join-Path $publish 'PixelArtStudio')
if ($LASTEXITCODE -ne 0) { throw "PixelArtStudio publish failed with exit code $LASTEXITCODE" }
foreach ($runtime in @('coreclr.dll','hostfxr.dll','hostpolicy.dll','PresentationFramework.dll','Microsoft.AspNetCore.Server.Kestrel.Core.dll')) {
    if (-not (Test-Path -LiteralPath (Join-Path $publish $runtime))) {
        throw "Self-contained runtime missing: $runtime"
    }
}
& $CompilerPath "/DSourceDir=$source" "/DPublishDir=$publish" "/DInstallerOutput=$output" "/DAppVersion=$version" (Join-Path $PSScriptRoot 'PixelTool.iss')
if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed with exit code $LASTEXITCODE" }
# Remove only this invocation's generated payload, never the source build directory.
$resolvedPayload = (Resolve-Path -LiteralPath $publish).Path
if (-not $resolvedPayload.StartsWith($output.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase) -or
    (Split-Path $resolvedPayload -Leaf) -notmatch '^Payload-[0-9a-f]{32}$') {
    throw "Refusing to clean unexpected payload path: $resolvedPayload"
}
Remove-Item -LiteralPath $resolvedPayload -Recurse -Force
