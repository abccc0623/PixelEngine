$ErrorActionPreference = 'Stop'
$solution = Split-Path $PSScriptRoot -Parent
function ReadSource($relative) {
    Get-Content -LiteralPath (Join-Path $solution $relative) -Raw -Encoding UTF8
}
function Require($condition, $message) {
    if (-not $condition) { throw $message }
}
$header = ReadSource 'PixelEngine/Entity.h'
$native = ReadSource 'PixelEngine/Entity.cpp'
$binding = ReadSource 'PixelEngine/BindManager.cpp'
$setting = ReadSource 'PixelEngine/Setting.lua'
$generator = ReadSource 'PixelEngine/LuaStaticCreate.cpp'
$lua = ReadSource '../Survival/Engine/Entity.lua'
foreach ($name in @('CreateCamera','Create2DImage')) {
    Require ($header.Contains('Entity_' + $name + '(') -and $binding.Contains('GeGlobalMethodInfo(&Entity_' + $name + ')')) "Native API and binding required: $name"
    Require (-not $setting.Contains('function Entity.' + $name + '(')) "Setting.lua must not shadow native $name"
    Require ($lua.Contains('return dll.Entity_' + $name + '(')) "Lua wrapper must invoke native $name"
}
$camera = [regex]::Match($native, 'unsigned int Entity_CreateCamera\(\)\s*\{([^}]+)\}').Groups[1].Value
Require ($camera.Contains('Entity_CreateEmpty()') -and $camera.Contains('Transform_Add(id)') -and $camera.Contains('Camera_Add(id)') -and $camera.Contains('return id;')) 'Camera creation must return the ID with Transform and Camera attached'
Require ($native.Contains('UIImageData* Entity_Create2DImage') -and $native.Contains('UIImage_Add(id)') -and $native.Contains('UIImage_SetTexture(id, textureName)') -and $native.Contains('return image;')) 'Image creation must set texture and return UIImageData'
Require ($lua.IndexOf('require("UIImage")') -lt $lua.IndexOf('ffi.cdef') -and $generator.Contains('require(\"UIImage\")')) 'UIImageData must be loaded before the Entity FFI declaration'
foreach ($name in @('GetValue','SetValue')) {
    Require (-not $native.Contains('Entity_' + $name + '(') -and -not $header.Contains('Entity_' + $name + '(')) "Removed helper must not remain in native API: $name"
}
Write-Output 'PASS: Native Entity creation APIs, bindings, Lua wrappers and obsolete helper removal (static checks only)'
