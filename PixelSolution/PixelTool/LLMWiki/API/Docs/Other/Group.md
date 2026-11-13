# PixelEngine Lua API

## Group

### 언제 사용하는지
Scene 안의 객체 ID를 이름별로 묶고 조회할 때 사용합니다.

### 최소 예제
```lua
local count = Group.GetCount("Enemies")
```

### 주의사항
- `GetID` 인덱스는 0부터 시작합니다. Clear는 멤버십만 지우며 객체를 파괴하지 않습니다.
