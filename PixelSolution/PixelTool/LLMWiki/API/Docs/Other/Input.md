# PixelEngine Lua API

## Input

### 언제 사용하는지
키의 유지·누름·뗌과 마우스 위치를 확인할 때 사용합니다. `Key`, `KeyDown`, `KeyUp`을 구분합니다.

### 최소 예제
```lua
local pressed = Input.KeyDown(Keycode.W)
```

### 주의사항
- 사용 가능한 키 상수는 `Lua/Enum.lua`의 `Keycode` 선언에서 확인합니다. 문자열 `"W"`나 Windows 가상 키 번호를 대신 쓰지 마세요.
