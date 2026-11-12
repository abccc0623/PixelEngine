# PixelEngine Lua component

## Rigidbody2D

### 언제 사용하는지
2D 객체에 충돌체와 물리 움직임을 추가할 때 사용합니다. 충돌체 생성, 운동 유형, 속도, 센서, 축 잠금 API가 등록되어 있습니다.

### 최소 예제
```lua
function Script:Spawn()
    self.body = Rigidbody2D.Add(self.ID)
end
```

### 주의사항
- 기본 운동 유형은 Kinematic, 센서는 `true`입니다. 중력과 단단한 바닥이 필요하면 운동 유형과 센서를 명시적으로 설정하세요.
- `SetLockPosition/Rotation`의 `true`는 해당 축 잠금입니다. `SetVelocity`는 전체 속도 벡터를 교체하며 등록된 속도 조회 API는 없습니다.
- Transform 위치 변경이 물리 위치도 바꾼다고 가정하지 마세요. `SetPosition`, `SetRotation`, `SetVelocity`를 확인하세요.
- `SetFriction`과 `SetRestitution`은 등록되어 있지만 현재 네이티브 함수 본문이 비어 있습니다.
- Lua 충돌 콜백은 상대 객체 ID를 받으며 접촉 법선은 제공하지 않습니다.

