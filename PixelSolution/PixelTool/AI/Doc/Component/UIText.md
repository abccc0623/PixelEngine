# PixelEngine Lua component

## UIText

### 언제 사용하는지
화면 UI의 글자·폰트·줄바꿈·피벗을 설정할 때 사용합니다. `SetText`, `SetFont`, `SetTextSize`, `SetWrapWidth`, `SetColor` 등이 등록되어 있습니다.

### 최소 예제
```lua
function Script:Spawn()
    self.text = UIText.Add(self.ID)
end
```

### 주의사항
- `SetTextSize`의 값은 크기 배율입니다.
- `SetWrapWidth`가 0 이하이면 줄바꿈 너비 제한이 꺼집니다.
- 줄바꿈 너비는 줄 높이 설정이 아닙니다.
- 색상 RGBA 값은 0~255입니다.

