$ErrorActionPreference = 'Stop'
$solution = Split-Path $PSScriptRoot -Parent
function Require($condition, $message) {
    if (-not $condition) { throw $message }
}
function ReadSource($path) {
    return Get-Content -LiteralPath (Join-Path $solution $path) -Encoding UTF8 -Raw
}
$header = ReadSource 'PixelGraphics/Export/RenderringData.h'
Require ($header -eq (ReadSource 'PixelGraphics/HeaderFile/PixelGraphicsHeader/RenderringData.h')) 'Graphics header mirror differs'
Require ($header -eq (ReadSource 'HeaderFile/PixelGraphicsHeader/RenderringData.h')) 'Engine import header differs'
$textStruct = [regex]::Match($header, 'struct TextData\s*\{([^}]+)\}', 'Singleline').Groups[1].Value
$spriteStruct = [regex]::Match($header, 'struct SpriteData\s*\{([^}]+)\}', 'Singleline').Groups[1].Value
Require ($textStruct.Contains('float wrapWidth;')) 'Text rendering packet has no wrap width'
Require (-not $spriteStruct.Contains('wrapWidth')) 'Wrap width must not change SpriteData'
$component = ReadSource 'PixelEngine/UIText.h'
Require ($component.Contains('float wrapWidth = 0.0f;')) 'Wrapping must be disabled by default'
Require ($component.Contains('UIText_SetWrapWidth(unsigned int id, float width)')) 'Native API declaration missing'
$api = ReadSource 'PixelEngine/UIText.cpp'
Require ($api.Contains('std::isfinite(width) && width > 0.0f ? width : 0.0f')) 'Invalid/nonpositive widths must disable wrapping'
Require ($api.Contains('data == nullptr || graphics == nullptr')) 'Missing component guard'
Require ($api.Contains('graphics->renderingData.text.wrapWidth = data->wrapWidth;')) 'Native setter must update rendering packet'
$system = ReadSource 'PixelEngine/UITextSystem.cpp'
Require ($system.Contains('target.wrapWidth = data->wrapWidth;')) 'System must preserve wrap width'
$binding = ReadSource 'PixelEngine/BindManager.cpp'
Require ($binding.Contains('GeGlobalMethodInfo(&UIText_SetWrapWidth)')) 'Lua generation metadata missing'
Require ($binding.Contains('name + "_SetWrapWidth", info_SetWrapWidth')) 'Lua metadata registration missing'
$renderer = ReadSource 'PixelGraphics/TextRenderer.cpp'
Require ($renderer.Contains('glyph.offsetX + glyph.width) * scale')) 'Wrap must use scaled glyph bounds'
Require ($renderer.Contains('data.text.wrapWidth > 0.0f && cursorX > 0.0f && cursorX + glyphRight > data.text.wrapWidth')) 'Width/first-glyph overflow rule changed'
$wrap = $renderer.IndexOf('cursorX + glyphRight > data.text.wrapWidth')
Require ($wrap -gt $renderer.IndexOf('ParseColorTag(text, textIndex, tagColor)')) 'Formatting tags must be processed before width checks'
Require ($wrap -lt $renderer.IndexOf('const float x0 = cursorX + glyph.offsetX')) 'Wrap must happen before emitting glyph geometry'
Require ($renderer.Substring($wrap, 250).Contains('textHeight += font->lineHeight * scale;')) 'Pivot block height must include wrapped lines'
$lua = Get-Content -LiteralPath (Join-Path $solution '../Survival/Engine/UIText.lua') -Encoding UTF8 -Raw
Require ($lua.Contains('void UIText_SetWrapWidth(unsigned int ID,float Width);')) 'Runtime FFI declaration missing'
Require ($lua.Contains('return dll.UIText_SetWrapWidth(self.thisID,Width)')) 'Runtime method forwarding missing'
Write-Output 'PASS: UIText wrap API, mirrored packet headers, metadata, renderer and Lua contracts (static checks only)'
