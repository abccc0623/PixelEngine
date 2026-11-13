# PixelEngine Lua component

## Renderer2D

### 언제 사용하는지
월드 객체의 텍스처·색·투명도를 설정할 때 사용합니다. `Add`, `Get`, `Has`, `SetTexture`, `SetColor`, `SetTransparent`가 등록되어 있습니다.

### 최소 예제
```lua
function Script:Spawn()
    self.renderer = Renderer2D.Add(self.ID)
    self.renderer:SetColor(255, 255, 255, 255)
end
```

### 주의사항
- `SetTexture`, `SetColor`, `SetTransparent`와 UV 필드 `TilingX/Y`, `OffsetX/Y`를 사용합니다.
- 색상 RGBA 값은 0~255입니다.
- 화면 UI에는 `UIImage` 또는 `UIText`를 사용합니다.

