# PixelEngine Lua API

## Entity

### 언제 사용하는지
객체를 만들고, 활성 상태를 바꾸거나 파괴할 때 사용합니다. `Create`, `CreateEmpty`, `CreateCamera`, `Create2DImage`, `Destroy`, `GetActive`, `SetActive`를 확인합니다.

### 최소 예제
```lua
local entityID = Entity.Create("Player")
```

### 주의사항
- `Create`는 스크립트의 `Spawn()`을 호출한 뒤 ID를 돌려줍니다. `CreateCamera()`는 ID, `Create2DImage()`는 UIImage 데이터를 반환합니다. 비활성화는 파괴가 아닙니다.
