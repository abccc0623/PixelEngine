# PixelEngine Lua API

## Tween

### 언제 사용하는지
객체의 위치·회전·크기 값을 시간에 따라 바꿀 때 사용합니다. `Move`, `PingPong`, `Pause`, `Resume`, `Kill`을 확인합니다.

### 최소 예제
```lua
Tween.Move(entityID, TweenProperty.PositionX, 0, 10, 1, false, TweenEase.Linear)
```

### 주의사항
- 같은 객체·속성의 새 Tween은 이전 것을 교체합니다. `PingPong`의 Duration은 한 방향의 시간입니다.
