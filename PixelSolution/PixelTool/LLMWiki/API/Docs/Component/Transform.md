# PixelEngine Lua component

## Transform

### 언제 사용하는지
객체의 위치·회전·크기를 지정하거나 부모와 자식 관계를 연결할 때 사용합니다. `Add`, `Get`, `Has`, `SetParent`, `SetChild`가 등록되어 있습니다.

### 최소 예제
```lua
function Script:Spawn()
    self.transform = Transform.Add(self.ID)
    self.transform.position.x = 10
end
```

### 주의사항
- `Transform.Get(ID)`는 이미 추가된 컴포넌트를 조회할 때만 사용하세요. 없는 객체에서 반환된 포인터의 `position` 등에 접근하면 LuaJIT FFI에서 메모리 접근 오류가 날 수 있습니다. 새 객체의 `Spawn()`에서는 `Transform.Add(self.ID)`로 먼저 추가하세요.
- 데이터 필드는 `position`, `rotation`, `scale`이며 벡터 좌표는 소문자 `x`, `y`, `z`를 씁니다.
- `scale`의 기본값은 `(1, 1, 1)`입니다.
- 자식 위치는 부모 기준입니다. 부모의 위치와 크기도 자식에 영향을 줍니다.
- 이미 연결된 자식을 중복 등록하거나 순환 관계를 만들지 마세요.

