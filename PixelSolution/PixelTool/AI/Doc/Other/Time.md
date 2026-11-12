# PixelEngine Lua API

## Time

### 언제 사용하는지
프레임 시간, 경과 시간과 일시정지 상태를 읽거나 바꿀 때 사용합니다.

### 최소 예제
```lua
local dt = Time.GetDeltaTime()
```

### 주의사항
- 일시정지 중 DeltaTime은 0이지만 활성 객체의 Update는 호출될 수 있습니다. Unscaled 시간과 TotalTime은 계속 진행합니다.
