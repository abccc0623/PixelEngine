$ErrorActionPreference = 'Stop'
$graphics = Join-Path $PSScriptRoot '../PixelGraphics'
function Require($condition, $message) {
    if (-not $condition) { throw $message }
}
$export = Get-Content (Join-Path $graphics 'Export/RenderringData.h') -Raw
$header = Get-Content (Join-Path $graphics 'HeaderFile/PixelGraphicsHeader/RenderringData.h') -Raw
Require ($export -eq $header) 'Graphics public header copies differ'
$scene = Get-Content (Join-Path $graphics 'ScenePass.cpp') -Raw
Require ($scene.Contains('return !data.transparent;')) 'Opaque/transparent partition missing'
Require ($scene.Contains('return leftView.z > rightView.z;')) 'Back-to-front camera-space sort missing'
Require ($scene.Contains('std::stable_sort(transparentBegin')) 'Equal-depth transparency order must be stable'
Require ($scene.Contains('GetDepthReadOnlyState()')) 'Transparent pass still writes depth'
Require ($scene.Contains('OMSetBlendState(nullptr, blendFactor, UINT_MAX)')) 'Opaque pass must disable blending'
$core = Get-Content (Join-Path $graphics 'GraphicsCore.cpp') -Raw
Require ($core.Contains('auto readOnlyDescription = enabledDescription;') -and $core.Contains('readOnlyDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;')) 'Depth test must stay enabled with writes disabled'
$ps = Get-Content (Join-Path $graphics 'Shaders/Static_ps.hlsl') -Raw
$vs = Get-Content (Join-Path $graphics 'Shaders/Static_vs.hlsl') -Raw
Require ($ps.Contains('float4 AlphaSettings;') -and $vs.Contains('float4 AlphaSettings;')) 'Shared shader buffer layout mismatch'
Require ($ps.Contains('finalColor.a <= 0.0f || finalColor.a < AlphaSettings.x')) 'Alpha cutoff must discard zero but keep faint transparency'
$pass = Get-Content (Join-Path $graphics 'PASS.cpp') -Raw
Require ($pass.Contains('r.transparent ? 0.0f : 0.1f')) 'Opaque threshold changed or transparent threshold still 0.1'
$ui = Get-Content (Join-Path $graphics 'UIPass.cpp') -Raw
Require ($ui.Contains('leftOrder < rightOrder')) 'UI Order sort changed'
$buffer = Get-Content (Join-Path $graphics 'CBufferResources.h') -Raw
Require ($buffer.Contains('sizeof(ObjectBuffer) == 224')) 'CPU constant-buffer layout guard missing'
$shaderFactory = Get-Content (Join-Path $graphics 'ShaderFactory.cpp') -Raw
Require ($shaderFactory.Contains('existing->second->byteWidth < requiredBytes')) 'VS/PS shared constant buffer cannot grow'
$client = Join-Path $PSScriptRoot '../../Survival/Asset/Script/Entity'
foreach($file in @('Bullet.pxm','Effects/HitEffect.pxm')) {
    $script = Get-Content (Join-Path $client $file) -Raw
    Require ($script.Contains('self.renderer:SetTransparent(true)')) "$file was not assigned to the transparent pass"
}
# Model checks: these verify the policy, not GPU output or native compilation.
$records = @(
    [pscustomobject]@{ Name='far'; Transparent=$true; Z=15 },
    [pscustomobject]@{ Name='opaque'; Transparent=$false; Z=0 },
    [pscustomobject]@{ Name='near'; Transparent=$true; Z=2 }
)
$ordered = @($records | Where-Object { -not $_.Transparent }) + @($records | Where-Object Transparent | Sort-Object Z -Descending)
Require (($ordered.Name -join ',') -eq 'opaque,far,near') 'Render policy ordering model failed'
foreach($alpha in @(0.01,0.05,0.099)) {
    Require (-not ($alpha -le 0 -or $alpha -lt 0)) 'Faint transparent pixel discarded'
    Require ($alpha -lt 0.1) 'Opaque cutout policy changed'
}
Write-Output 'PASS: source contracts and ordering/alpha policy model; native build/GPU rendering not tested'
