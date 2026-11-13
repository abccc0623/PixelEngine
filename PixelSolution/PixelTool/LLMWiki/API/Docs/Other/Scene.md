# PixelEngine Lua API

## Scene

### 언제 사용하는지
가져온 Scene을 실제 실행 장면으로 전환할 때 사용합니다.

### 최소 예제
```lua
Scene.Change("Main")
```

### 주의사항
- Scene 파일을 가져오기만 해서는 실행되지 않습니다. 기존 객체는 새 Scene으로 자동 이동하지 않습니다.
