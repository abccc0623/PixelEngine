# PixelEngine Lua component

## UISilder

### 언제 사용하는지
UI 슬라이더의 범위·값·크기·색·텍스처를 설정할 때 사용합니다. `SetRange`, `SetValue`, `SetSize`, `SetOrder` 등이 등록되어 있습니다.

### 최소 예제
```lua
function Script:Spawn()
    self.slider = UISilder.Add(self.ID)
end
```

### 주의사항
- 현재 엔진 등록 이름은 `UISilder`입니다. `UISlider`로 바꾸지 마세요.
- 배경과 채움의 색상 RGBA 값은 0~255입니다.

