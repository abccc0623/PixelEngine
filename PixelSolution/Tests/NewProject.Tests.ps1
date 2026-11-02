$ErrorActionPreference = 'Stop'
$solution = Split-Path $PSScriptRoot -Parent
$project = Get-Content -Raw -Encoding UTF8 (Join-Path $solution 'PixelTool/ProjectPathService.cs')
$graphics = Get-Content -Raw -Encoding UTF8 (Join-Path $solution 'PixelTool/Tool/GraphicsWindow/GraphicsWindow.cs')
$lua = Get-Content -Raw -Encoding UTF8 (Join-Path $solution 'PixelEngine/LuaManager.cpp')
$main = Get-Content -Raw -Encoding UTF8 (Join-Path $solution 'PixelTool/LuaCode/main.lua')
function Require($condition, $message) {
    if (-not $condition) { throw $message }
}
Require ($project.Contains('EnsureLayerMatrix(rootPath);')) 'Default matrix must exist before native engine initialization'
Require ($project.Contains('if (File.Exists(path)) return;')) 'Existing matrix must not be overwritten'
Require ($project.Contains('fileType = "LayerMatrix"') -and $project.Contains('new[] { "Default", "Player", "Ground" }')) 'Default matrix schema differs from Layer Settings'
Require ($project.Contains('GenerateProjectFilesPending |= !File.Exists')) 'Existing projects missing engine Lua must trigger Engine Update'
Require ($graphics.IndexOf('ProjectPathService.StartNewProject();') -gt $graphics.IndexOf('PixelEngineNative.EngineInitialize(')) 'Metadata generation needs the initialized engine'
Require ($graphics.IndexOf('ProjectPathService.StartNewProject();') -lt $graphics.IndexOf('PixelEngineNative.Asset_Import(')) 'Generate engine Lua before importing main'
Require ($lua.Contains('if (!UpdateFunction.valid())') -and $lua.Contains('scriptRoot / "Entity.lua"')) 'Fresh projects must defer Lua runtime setup until API files exist'
Require ($main.Contains('Entity.Create("BaseCamera")') -and -not $main.Contains('Engine.CreateEntity')) 'Main template uses an obsolete API'
Write-Output 'PASS: new-project initialization contracts (static checks only)'
