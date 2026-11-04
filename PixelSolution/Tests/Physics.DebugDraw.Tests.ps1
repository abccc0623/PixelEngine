$ErrorActionPreference = 'Stop'
$solution = Split-Path $PSScriptRoot -Parent
function ReadSource($relative) {
    Get-Content -LiteralPath (Join-Path $solution $relative) -Raw -Encoding UTF8
}
function Require($condition, $message) {
    if (-not $condition) { throw $message }
}
$physics = ReadSource 'PixelEngine/PhysManager.cpp'
$system = ReadSource 'PixelEngine/Rigidbody2DSystem.cpp'
$order = ReadSource 'PixelEngine/SystemManager.cpp'
Require (-not ($physics -match 'DrawLine\(w\[\d\]\.GetX\(\), w\[\d\]\.GetY\(\), 0,')) 'Box edges must use transformed Z coordinates'
Require ($physics.Contains('DrawColliderOutline') -and $physics.Contains('GetRotation()')) 'Preview and runtime must share collider outline transforms'
foreach ($axis in @('x','y','z')) {
    Require ($physics.Contains('transformData->rotation.' + $axis + ' =')) "Physics rotation must sync $axis"
}
$editor = $system.Substring($system.IndexOf('void ECS::Rigidbody2DSystem::EditorUpdate'))
Require ($editor.Contains('DebugDrawPreview') -and -not $editor.Contains('CreateRigidbody') -and -not $editor.Contains('SyncPhysics')) 'Editor preview must draw without creating or simulating physics bodies'
Require ($order.IndexOf('new Rigidbody2DSystem') -lt $order.IndexOf('new TransformSystem')) 'World matrices must update after physics synchronization in the same frame'
Require ($physics.Contains('Engine == nullptr || !Engine->IsTool')) 'Automatic physics outlines must stay editor-only'
Write-Output 'PASS: Physics outlines, editor preview and rotation synchronization (static checks only)'
