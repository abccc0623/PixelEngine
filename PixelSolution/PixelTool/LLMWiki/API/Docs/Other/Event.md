# PixelEngine Lua API

## Event

### 언제 사용하는지
이벤트 이름에 객체 함수를 등록하고 호출할 때 사용합니다.

### 최소 예제
```lua
Event.Call("GameStart")
```

### 주의사항
- `Bind`는 호출할 함수의 이름을 문자열로 받습니다. `Call` 전에 실제 함수와 이벤트가 등록되어 있어야 합니다.
