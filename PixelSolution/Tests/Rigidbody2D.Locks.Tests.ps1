$ErrorActionPreference = 'Stop'
$solution = Split-Path $PSScriptRoot -Parent
function ReadSource($relative) {
    Get-Content -LiteralPath (Join-Path $solution $relative) -Raw -Encoding UTF8
}
function Require($condition, $message) {
    if (-not $condition) { throw $message }
}
$api = ReadSource 'PixelEngine/Rigidbody2D.cpp'
$header = ReadSource 'PixelEngine/Rigidbody2D.h'
$binding = ReadSource 'PixelEngine/BindManager.cpp'
$creation = ReadSource 'PixelEngine/PhysManager.cpp'
$physics = ReadSource 'PixelEngine/PhysicsFunction.cpp'
$data = ReadSource 'PixelEngine/Physics2DData.h'
$lua = ReadSource '../Survival/Engine/Rigidbody2D.lua'
foreach ($name in @('SetLockPosition', 'SetLockRotation')) {
    Require ($header -match ('(?m)^extern "C".*bool Rigidbody2D_' + $name + '\(')) "Missing exported $name"
    Require ($binding.Contains('GeGlobalMethodInfo(&Rigidbody2D_' + $name + ')')) "Missing binding for $name"
    Require ($lua.Contains('function Rigidbody2DData:' + $name + '(X,Y,Z)')) "Missing Lua method for $name"
}
Require ($data.Contains('allowedDOFs = JPH::EAllowedDOFs::All')) 'Existing default freedom must be preserved'
Require ($api.Contains('data->allowedDOFs & ~(axisX | axisY | axisZ)') -and $api.Contains('if (!x) allowed |= axisX;') -and $api.Contains('if (!y) allowed |= axisY;') -and $api.Contains('if (!z) allowed |= axisZ;')) 'Locks must preserve the other axis group and true must mean locked'
Require ($creation.Contains('bodySettings.mAllowedDOFs = physics->allowedDOFs') -and $creation.Contains('bodySettings.mAllowDynamicOrKinematic = true')) 'Creation must retain locks and allow later unlocking'
Require ($physics.Contains('JPH::BodyLockWrite lock') -and $physics.Contains('body.GetShape()->GetMassProperties()') -and $physics.Contains('motion->SetAngularVelocityClamped') -and $physics.Contains('motion->SetLinearVelocityClamped')) 'Runtime changes must lock the body, rebuild mass properties and remove locked velocity'
Require ($physics.Contains('fullyLocked ? JPH::EMotionType::Static : motionType') -and $physics.Contains('fullyLocked ? JPH::EAllowedDOFs::All : allowedDOFs')) 'All six locked axes must not pass unsupported None DOFs into Jolt'
Require ($api.Contains('phys->SetAllowedDOFs(data1->bodyID, data1->allowedDOFs, data1->Motiontype)')) 'Motion type changes must retain axis locks'
Write-Output 'PASS: Rigidbody2D axis lock exports, bindings, creation, runtime and all-locked safety (static checks only)'
