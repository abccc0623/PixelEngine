# PixelEngine Lua API

## Engine

### 언제 사용하는지
게임 배경색이나 클라이언트 창 크기를 설정할 때 사용합니다.

### 최소 예제
```lua
Engine.BackgroundColor(0, 0, 0)
```

### 주의사항
- `SetWindowSize`는 창 크기만 바꾸며 1920×1080 UI 가상 캔버스를 바꾸지 않습니다. PixelTool에서는 창 크기 설정이 무시됩니다.
