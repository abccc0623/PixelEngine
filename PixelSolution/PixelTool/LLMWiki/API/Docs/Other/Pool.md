# PixelEngine Lua API

## Pool

### 언제 사용하는지
같은 스크립트 객체를 재사용할 때 사용합니다. `Active`, `Disable`, `GetActiveCount`, `GetActiveID`를 확인합니다.

### 최소 예제
```lua
local entityID = Pool.Active("Enemy")
```

### 주의사항
- 재사용 시 Spawn은 다시 실행되지 않습니다. 상태를 직접 초기화하세요. 조회 인덱스는 0부터 시작하고 Clear는 객체를 파괴하지 않습니다.
