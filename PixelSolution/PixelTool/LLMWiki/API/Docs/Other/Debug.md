# PixelEngine Lua API

## Debug

### 언제 사용하는지
게임 로그와 디버그 선을 출력할 때 사용합니다.

### 최소 예제
```lua
Debug.Log("started")
```

### 주의사항
- 경고와 오류에는 `LogWarning`, `LogError`를 사용합니다. `LogInfo`는 임의로 만들지 마세요. 로그만으로 화면 동작을 검증했다고 주장하지 마세요.
