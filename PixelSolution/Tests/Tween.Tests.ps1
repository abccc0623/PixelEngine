$ErrorActionPreference = 'Stop'
$solution = Split-Path $PSScriptRoot -Parent
function Require($condition, $message) {
    if (-not $condition) { throw $message }
}
function ReadSource($path) {
    return Get-Content -LiteralPath (Join-Path $solution $path) -Encoding UTF8 -Raw
}
$properties = @('PositionX','PositionY','PositionZ','RotationX','RotationY','RotationZ','ScaleX','ScaleY','ScaleZ')
$header = ReadSource 'PixelEngine/Tween.h'
$manager = ReadSource 'PixelEngine/TweenManager.cpp'
$managerHeader = ReadSource 'PixelEngine/TweenManager.h'
$binding = ReadSource 'PixelEngine/BindManager.cpp'
$scene = ReadSource 'PixelEngine/SceneObject.cpp'
$luaEnum = ReadSource '../Survival/Engine/Enum.lua'
$nativeEnum = [regex]::Match($header, 'enum class TweenProperty : int\s*\{([^}]+)\}', 'Singleline').Groups[1].Value
$easeNames = @('Linear','In','Out','InOut')
$easeEnum = [regex]::Match($header, 'enum class TweenEase : int\s*\{([^}]+)\}', 'Singleline').Groups[1].Value
$easeEntries = @([regex]::Matches($easeEnum, '\b\w+\b') | ForEach-Object { $_.Value })
Require (($easeEntries -join ',') -eq ($easeNames -join ',')) 'Native easing enum order differs'
for ($i = 0; $i -lt $easeNames.Count; $i++) {
    Require ($luaEnum.Contains("$($easeNames[$i]) = $i,")) 'Runtime easing enum mismatch'
    Require ($binding.Contains("AddEnum(globalEnum, `"$($easeNames[$i])`");")) 'Generated easing enum missing'
}
Require ($manager.Contains('ease < 0 || ease > static_cast<int>(TweenEase::InOut)')) 'Invalid easing must be rejected'
Require ($manager.Contains('case TweenEase::In: t = t * t;')) 'Ease In curve missing'
Require ($manager.Contains('case TweenEase::Out: t = t * (2.0 - t);')) 'Ease Out curve missing'
Require ($manager.Contains('2.0 * t * t : 1.0 - 2.0 * (1.0 - t) * (1.0 - t)')) 'Ease InOut curve missing'
$generator = ReadSource 'PixelEngine/LuaStaticCreate.cpp'
Require ($generator.Contains('Ease = Ease or 0')) 'Regeneration must preserve the default easing'
$entries = @([regex]::Matches($nativeEnum, '\b\w+\b') | ForEach-Object { $_.Value })
Require (($entries -join ',') -eq ($properties -join ',')) 'Native property enum order differs'
for ($i = 0; $i -lt $properties.Count; $i++) {
    $property = $properties[$i]
    $member = @('position','rotation','scale')[[math]::Floor($i / 3)]
    $axis = @('x','y','z')[$i % 3]
    Require ($manager.Contains("case TweenProperty::${property}: return &transform->${member}.${axis};")) "Wrong target mapping for $property"
    Require ($binding.Contains("AddEnum(globalEnum, `"$property`");")) "Missing generated property $property"
    Require ($luaEnum.Contains("$property = $i,")) "Runtime enum mismatch for $property"
}
Require ($manager.Contains('static_cast<std::uint64_t>(id) << 32')) 'Object/property keys must not collide'
Require ($manager.Contains('tweens.insert_or_assign(')) 'Repeated ID/property must replace existing state'
Require ($manager.Contains('duration <= 0.0f') -and $manager.Contains('if (minValue > maxValue) return false;')) 'Invalid durations and ping-pong ranges must be rejected'
Require ($manager.Contains('!std::isfinite(startValue)') -and $manager.Contains('!std::isfinite(duration)')) 'Non-finite registration values must be rejected'
Require ($manager.Contains('return Start(registry,id,property,startValue,endValue,duration,useUnscaledTime,false,ease);')) 'Move must share registration without restricting start/end direction'
Require ($manager.Contains('!tween.PingPong && tween.Elapsed + step >= duration')) 'One-shot moves must complete on reaching or overshooting duration'
$complete = $manager.IndexOf('*target = tween.EndValue;')
Require ($complete -ge 0 -and $manager.Substring($complete,160).Contains('it = tweens.erase(it);')) 'Completion must set the exact end value before removing the tween'
Require ($manager.Contains('tween.Elapsed = std::fmod(tween.Elapsed + step, duration * 2.0);')) 'Ping-pong phase must wrap over a full round trip'
Require ($manager.Contains('phase <= 1.0 ? phase : 2.0 - phase')) 'Return leg must run backwards'
Require ($manager.Contains('registry->Get<TransformData>(tween.ID)')) 'Update must resolve current components by ID'
$inactive = $manager.IndexOf('if (!Entity_GetActive(tween.ID))')
Require ($inactive -gt $manager.IndexOf('if (target == nullptr)')) 'Deleted targets must be removed before checking activity'
Require ($inactive -lt $manager.IndexOf('const float step = tween.UseUnscaledTime')) 'Inactive tweens must not accumulate either clock'
Require ($manager.Substring($inactive,100).Contains('++it;') -and $manager.Substring($inactive,100).Contains('continue;')) 'Skip only the inactive tween without blocking other objects'
Require (-not $managerHeader.Contains('TransformData*')) 'Manager must not retain component pointers'
Require ($manager.Contains('it = tweens.erase(it);')) 'Missing component must cancel the tween'
Require (-not ($manager -match 'sol::|lua_State|CallFunction')) 'Tween update must not re-enter Lua'
Require ($manager.Contains('tween.UseUnscaledTime ? unscaledDeltaTime : deltaTime')) 'Scaled/unscaled clock selection missing'
Require ($manager.Contains('!tween.Paused && std::isfinite(step) && step > 0.0f')) 'Paused/invalid time steps must not advance'
Require ($scene.Contains('tween = new ECS::TweenManager();')) 'Manager must be owned by each scene'
Require ($scene.IndexOf('tween->Update(') -lt $scene.IndexOf('system->Update(registry)')) 'Tween values must be applied before transform matrices'
Require ($scene.IndexOf('tween->RemoveFromAll(id)') -lt $scene.IndexOf('registry->Remove(id)')) 'Object deletion must cancel all its tweens first'
Require ($scene.IndexOf('tween->Clear()') -lt $scene.IndexOf('delete registry')) 'Scene release must clear tweens before components'
$projectText = ReadSource 'PixelEngine/PixelEngine.vcxproj'
$project = [xml]$projectText
foreach ($file in @('Tween.cpp','TweenManager.cpp','Tween.h','TweenManager.h')) {
    Require ($projectText.Contains("Include=`"$file`"")) "Project entry missing for $file"
}
foreach ($method in @('PingPong','Move','Pause','Resume','Kill')) {
    Require ($binding.Contains("`"Tween_$method`"")) "Generated global API missing: $method"
}
$loader = ReadSource '../Survival/Engine/EngineGenerate.lua'
Require ($loader.Contains('require("Tween")')) 'Runtime module loader missing Tween'
Write-Output 'PASS: Tween scene lifetime, enum/target mappings, clock/phase formulas, project and Lua metadata contracts (static checks only)'
