# PixelEngine Lua API

## Scene

### 언제 사용하는지
서로 다른 게임 장면을 파일로 정의하고 전환할 때 사용합니다. Scene은 `Asset` 아래의 `.scene` 파일로 만들고 `Asset.Import`로 등록한 뒤 `Scene.Change`로 선택합니다.

### 최소 예제
```lua
-- Asset/main.lua의 Main() 안
Asset.Import("Asset/Script/Player.pxm")
Asset.Import("Asset/Scene/Level.scene")
Scene.Change("Level")
```

```lua
-- Asset/Scene/Level.scene
local Script = {}

function Script:Start()
    Entity.Create("Player")
end

return Script
```

### 주의사항
- Lua에 `Scene.Create`는 등록되어 있지 않습니다. 새 장면은 `.scene` 파일을 만들고 확장자까지 포함해 `Asset.Import`로 가져옵니다. `Scene.Change`에는 파일의 확장자를 제외한 이름을 전달합니다.
- Scene 파일은 테이블을 반환해야 합니다. 가져오기만 해서는 실행되지 않으며, `Scene.Change`로 전환한 뒤 `Start()`에서 해당 장면의 객체를 만듭니다.
- 전환 시 기존 Scene의 객체는 새 Scene으로 자동 이동하지 않습니다. `Scene.Change`는 전환을 예약하므로 호출 직후 새 Scene의 객체가 있다고 가정하지 않습니다.
