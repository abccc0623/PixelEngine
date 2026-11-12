# PixelEngine Lua API

## PVector3

### 언제 사용하는지
3차원 위치·회전·크기와 벡터 연산에 사용합니다.

### 최소 예제
```lua
local position = PVector3(0, 0, 0)
```

### 주의사항
- 좌표 필드는 소문자 `x`, `y`, `z`입니다. `Normalize`, `Length`, `Distance`, `Dot`, `Cross`, `Direction`, `Lerp` 등의 실제 서명은 프로젝트 선언에서 확인하세요.
